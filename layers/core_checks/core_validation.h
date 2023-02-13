/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
#include "core_checks/shader_validation.h"
#include "core_error_location.h"
#include "qfo_transfer.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/video_session_state.h"
#include "sync/sync_vuid_maps.h"

// Set of VUID that need to go between drawdispatch_validation.cpp and rest of CoreChecks
struct DrawDispatchVuid {
    const char* pipeline_bound = kVUIDUndefined;
    const char* index_binding = kVUIDUndefined;
    const char* vertex_binding = kVUIDUndefined;
    const char* vertex_binding_null = kVUIDUndefined;
    const char* compatible_pipeline = kVUIDUndefined;
    const char* render_pass_compatible = kVUIDUndefined;
    const char* subpass_index = kVUIDUndefined;
    const char* sample_location = kVUIDUndefined;
    const char* linear_filter_sampler = kVUIDUndefined;
    const char* linear_mipmap_sampler = kVUIDUndefined;
    const char* cubic_sampler = kVUIDUndefined;
    const char* indirect_protected_cb = kVUIDUndefined;
    const char* indirect_contiguous_memory = kVUIDUndefined;
    const char* indirect_count_contiguous_memory = kVUIDUndefined;
    const char* indirect_buffer_bit = kVUIDUndefined;
    const char* indirect_count_buffer_bit = kVUIDUndefined;
    const char* indirect_count_offset = kVUIDUndefined;
    const char* viewport_count = kVUIDUndefined;
    const char* scissor_count = kVUIDUndefined;
    const char* viewport_scissor_count = kVUIDUndefined;
    const char* primitive_topology_class = kVUIDUndefined;
    const char* primitive_topology_class_ds3 = kVUIDUndefined;
    const char* dynamic_primitive_topology = kVUIDUndefined;
    const char* corner_sampled_address_mode = kVUIDUndefined;
    const char* imageview_atomic = kVUIDUndefined;
    const char* push_constants_set = kVUIDUndefined;
    const char* image_subresources_render_pass_write = kVUIDUndefined;
    const char* image_subresources_subpass_read = kVUIDUndefined;
    const char* image_subresources_subpass_write = kVUIDUndefined;
    const char* descriptor_valid = kVUIDUndefined;
    const char* sampler_imageview_type = kVUIDUndefined;
    const char* sampler_implicitLod_dref_proj = kVUIDUndefined;
    const char* sampler_bias_offset = kVUIDUndefined;
    const char* vertex_binding_attribute = kVUIDUndefined;
    const char* dynamic_state_setting_commands = kVUIDUndefined;
    const char* rasterization_samples = kVUIDUndefined;
    const char* msrtss_rasterization_samples = kVUIDUndefined;
    const char* unprotected_command_buffer = kVUIDUndefined;
    const char* protected_command_buffer = kVUIDUndefined;
    const char* ray_query_protected_cb = kVUIDUndefined;
    // TODO: Some instance values are in VkBuffer. The validation in those Cmds is skipped.
    const char* max_multiview_instance_index = kVUIDUndefined;
    const char* img_filter_cubic = kVUIDUndefined;
    const char* filter_cubic = kVUIDUndefined;
    const char* filter_cubic_min_max = kVUIDUndefined;
    const char* viewport_count_primitive_shading_rate = kVUIDUndefined;
    const char* patch_control_points = kVUIDUndefined;
    const char* rasterizer_discard_enable = kVUIDUndefined;
    const char* depth_bias_enable = kVUIDUndefined;
    const char* logic_op = kVUIDUndefined;
    const char* primitive_restart_enable = kVUIDUndefined;
    const char* vertex_input_binding_stride = kVUIDUndefined;
    const char* vertex_input = kVUIDUndefined;
    const char* blend_enable = kVUIDUndefined;
    const char* dynamic_discard_rectangle = kVUIDUndefined;
    const char* dynamic_color_write_enable = kVUIDUndefined;
    const char* dynamic_color_write_enable_count = kVUIDUndefined;
    const char* dynamic_rendering_view_mask = kVUIDUndefined;
    const char* dynamic_rendering_color_count = kVUIDUndefined;
    const char* dynamic_rendering_color_formats = kVUIDUndefined;
    const char* dynamic_rendering_depth_format = kVUIDUndefined;
    const char* dynamic_rendering_stencil_format = kVUIDUndefined;
    const char* dynamic_rendering_fsr = kVUIDUndefined;
    const char* dynamic_rendering_fdm = kVUIDUndefined;
    const char* dynamic_rendering_color_sample = kVUIDUndefined;
    const char* dynamic_rendering_depth_sample = kVUIDUndefined;
    const char* dynamic_rendering_stencil_sample = kVUIDUndefined;
    const char* dynamic_rendering_multi_sample = kVUIDUndefined;
    const char* dynamic_rendering_06189 = kVUIDUndefined;
    const char* dynamic_rendering_06190 = kVUIDUndefined;
    const char* dynamic_rendering_06198 = kVUIDUndefined;
    const char* dynamic_rendering_07285 = kVUIDUndefined;
    const char* dynamic_rendering_07286 = kVUIDUndefined;
    const char* dynamic_rendering_07287 = kVUIDUndefined;
    const char* image_view_access_64 = kVUIDUndefined;
    const char* image_view_access_32 = kVUIDUndefined;
    const char* buffer_view_access_64 = kVUIDUndefined;
    const char* buffer_view_access_32 = kVUIDUndefined;
    const char* storage_image_read_without_format = kVUIDUndefined;
    const char* storage_image_write_without_format = kVUIDUndefined;
    const char* storage_texel_buffer_read_without_format = kVUIDUndefined;
    const char* storage_texel_buffer_write_without_format = kVUIDUndefined;
    const char* storage_image_write_texel_count = kVUIDUndefined;
    const char* storage_texel_buffer_write_texel_count = kVUIDUndefined;
    const char* depth_compare_sample = kVUIDUndefined;
    const char* depth_read_only = kVUIDUndefined;
    const char* stencil_read_only = kVUIDUndefined;
    const char* dynamic_sample_locations = kVUIDUndefined;
    const char* dynamic_tessellation_domain_origin = kVUIDUndefined;
    const char* dynamic_depth_clamp_enable = kVUIDUndefined;
    const char* dynamic_polygon_mode = kVUIDUndefined;
    const char* dynamic_rasterization_samples = kVUIDUndefined;
    const char* dynamic_sample_mask = kVUIDUndefined;
    const char* dynamic_alpha_to_coverage_enable = kVUIDUndefined;
    const char* dynamic_alpha_to_one_enable = kVUIDUndefined;
    const char* dynamic_logic_op_enable = kVUIDUndefined;
    const char* dynamic_color_blend_enable = kVUIDUndefined;
    const char* dynamic_color_blend_equation = kVUIDUndefined;
    const char* dynamic_color_write_mask = kVUIDUndefined;
    const char* dynamic_rasterization_stream = kVUIDUndefined;
    const char* dynamic_conservative_rasterization_mode = kVUIDUndefined;
    const char* dynamic_extra_primitive_overestimation_size = kVUIDUndefined;
    const char* dynamic_depth_clip_enable = kVUIDUndefined;
    const char* dynamic_sample_locations_enable = kVUIDUndefined;
    const char* dynamic_color_blend_advanced = kVUIDUndefined;
    const char* dynamic_provoking_vertex_mode = kVUIDUndefined;
    const char* dynamic_line_rasterization_mode = kVUIDUndefined;
    const char* dynamic_line_stipple_enable = kVUIDUndefined;
    const char* dynamic_depth_clip_negative_one_to_one = kVUIDUndefined;
    const char* dynamic_viewport_w_scaling_enable = kVUIDUndefined;
    const char* dynamic_viewport_swizzle = kVUIDUndefined;
    const char* dynamic_coverage_to_color_enable = kVUIDUndefined;
    const char* dynamic_coverage_to_color_location = kVUIDUndefined;
    const char* dynamic_coverage_modulation_mode = kVUIDUndefined;
    const char* dynamic_coverage_modulation_table_enable = kVUIDUndefined;
    const char* dynamic_coverage_modulation_table = kVUIDUndefined;
    const char* dynamic_coverage_reduction_mode = kVUIDUndefined;
    const char* dynamic_representative_fragment_test_enable = kVUIDUndefined;
    const char* dynamic_shading_rate_image_enable = kVUIDUndefined;
    const char* dynamic_viewport = kVUIDUndefined;
    const char* dynamic_scissor = kVUIDUndefined;
    const char* dynamic_depth_bias = kVUIDUndefined;
    const char* dynamic_line_width = kVUIDUndefined;
    const char* dynamic_line_stipple_ext = kVUIDUndefined;
    const char* dynamic_blend_constants = kVUIDUndefined;
    const char* dynamic_depth_bounds = kVUIDUndefined;
    const char* dynamic_stencil_compare_mask = kVUIDUndefined;
    const char* dynamic_stencil_write_mask = kVUIDUndefined;
    const char* dynamic_stencil_reference = kVUIDUndefined;
    const char* dynamic_state_inherited = kVUIDUndefined;
    const char* primitives_generated = kVUIDUndefined;
    const char* primitives_generated_streams = kVUIDUndefined;
    const char* mesh_shader_stages = kVUIDUndefined;
    const char* invalid_mesh_shader_stages = kVUIDUndefined;
    const char* missing_mesh_shader_stages = kVUIDUndefined;
    const char* descriptor_buffer_bit_set = kVUIDUndefined;
    const char* descriptor_buffer_bit_not_set = kVUIDUndefined;
    const char* descriptor_buffer_set_offset_missing = kVUIDUndefined;
    const char* image_view_dim = kVUIDUndefined;
    const char* image_view_numeric_format = kVUIDUndefined;
    const char* stippled_rectangular_lines = kVUIDUndefined;
    const char* stippled_bresenham_lines = kVUIDUndefined;
    const char* stippled_smooth_lines = kVUIDUndefined;
    const char* stippled_default_strict = kVUIDUndefined;
};

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
    const char* vuid_no_active_in_vc_scope = kVUIDUndefined;
    const char* vuid_result_status_profile_in_vc_scope = kVUIDUndefined;
    const char* vuid_vc_scope_query_type = kVUIDUndefined;
};

struct ValidateEndQueryVuids {
    const char* vuid_active_queries = kVUIDUndefined;
    const char* vuid_protected_cb = kVUIDUndefined;
};

struct SubresourceRangeErrorCodes {
    const char *base_mip_err, *mip_count_err, *base_layer_err, *layer_count_err;
};

typedef vvl::unordered_map<const IMAGE_STATE*, std::optional<GlobalImageLayoutRangeMap>> GlobalImageLayoutMap;

// Much of the data stored in CMD_BUFFER_STATE is only used by core validation, and is
// set up by Record calls in class CoreChecks. Because both the state tracker and
// core methods must lock CMD_BUFFER_STATE, it is possible for a Validate call to
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
class CORE_CMD_BUFFER_STATE : public CMD_BUFFER_STATE {
  public:
    CORE_CMD_BUFFER_STATE(ValidationStateTracker* dev_data, VkCommandBuffer cb, const VkCommandBufferAllocateInfo* pCreateInfo,
                          const COMMAND_POOL_STATE* cmd_pool)
        : CMD_BUFFER_STATE(dev_data, cb, pCreateInfo, cmd_pool) {}

    void RecordWaitEvents(CMD_TYPE cmd_type, uint32_t eventCount, const VkEvent* pEvents,
                          VkPipelineStageFlags2KHR src_stage_mask) override;
};

struct TimelineMaxDiffCheck {
    TimelineMaxDiffCheck(uint64_t value_, uint64_t max_diff_) : value(value_), max_diff(max_diff_) {}

    // compute the differents between 2 timeline values, without rollover if the difference is greater than INT64_MAX
    uint64_t AbsDiff(uint64_t a, uint64_t b) { return a > b ? a - b : b - a; }

    bool operator()(const SEMAPHORE_STATE::SemOp& op, bool is_pending) { return AbsDiff(value, op.payload) > max_diff; }

    uint64_t value;
    uint64_t max_diff;
};

class CoreChecks;
struct SemaphoreSubmitState {
    const CoreChecks* core;
    VkQueue queue;
    VkQueueFlags queue_flags;
    vvl::unordered_set<VkSemaphore> signaled_semaphores;
    vvl::unordered_set<VkSemaphore> unsignaled_semaphores;
    vvl::unordered_set<VkSemaphore> internal_semaphores;
    vvl::unordered_map<VkSemaphore, uint64_t> timeline_signals;
    vvl::unordered_map<VkSemaphore, uint64_t> timeline_waits;

    SemaphoreSubmitState(const CoreChecks* core_, VkQueue q_, VkQueueFlags queue_flags_)
        : core(core_), queue(q_), queue_flags(queue_flags_) {}

    bool CannotWait(const SEMAPHORE_STATE& semaphore_state) const {
        auto semaphore = semaphore_state.semaphore();
        return unsignaled_semaphores.count(semaphore) || (!signaled_semaphores.count(semaphore) && !semaphore_state.CanBeWaited());
    }
    VkQueue AnotherQueueWaits(const SEMAPHORE_STATE& semaphore_state) const {
        // spec (for 003871 but all submit functions have a similar VUID):
        // "When a semaphore wait operation for a binary semaphore is **executed**,
        // as defined by the semaphore member of any element of the pWaitSemaphoreInfos
        // member of any element of pSubmits, there must be no other queues waiting on the same semaphore"
        //
        // For binary semaphores there can be only 1 wait per signal so we just need to check that the
        // last operation isn't a wait. Prior waits will have been removed by prior signals by the time
        // this wait executes.
        auto last_op = semaphore_state.LastOp();
        if (last_op && !last_op->CanBeWaited() && last_op->queue && last_op->queue->Queue() != queue) {
            return last_op->queue->Queue();
        }
        return VK_NULL_HANDLE;
    }

    bool ValidateBinaryWait(const core_error::Location& loc, VkQueue queue, const SEMAPHORE_STATE& semaphore_state);
    bool ValidateWaitSemaphore(const core_error::Location& loc, VkSemaphore semaphore, uint64_t value);
    bool ValidateSignalSemaphore(const core_error::Location& loc, VkSemaphore semaphore, uint64_t value);

    bool CannotSignal(const SEMAPHORE_STATE& semaphore_state, VkQueue& other_queue) const {
        const auto semaphore = semaphore_state.semaphore();
        if (signaled_semaphores.count(semaphore)) {
            other_queue = queue;
            return true;
        }
        if (!unsignaled_semaphores.count(semaphore)) {
            const auto last_op = semaphore_state.LastOp();
            if (last_op && !last_op->CanBeSignaled()) {
                other_queue = last_op && last_op->queue ? last_op->queue->Queue() : VK_NULL_HANDLE;
                return true;
            }
        }
        return false;
    }

    bool CheckSemaphoreValue(const SEMAPHORE_STATE& semaphore_state, std::string& where, uint64_t& bad_value,
                             std::function<bool(const SEMAPHORE_STATE::SemOp&, bool is_pending)> compare_func) {
        auto current_signal = timeline_signals.find(semaphore_state.semaphore());
        // NOTE: for purposes of validation, duplicate operations in the same submission are not yet pending.
        if (current_signal != timeline_signals.end()) {
            SEMAPHORE_STATE::SemOp sig_op(SEMAPHORE_STATE::kSignal, nullptr, 0, current_signal->second);
            if (compare_func(sig_op, false)) {
                where = "current submit's signal";
                bad_value = sig_op.payload;
                return true;
            }
        }
        auto current_wait = timeline_waits.find(semaphore_state.semaphore());
        if (current_wait != timeline_waits.end()) {
            SEMAPHORE_STATE::SemOp wait_op(SEMAPHORE_STATE::kWait, nullptr, 0, current_wait->second);
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
    using Location = core_error::Location;
    using Func = core_error::Func;
    using Struct = core_error::Struct;
    using Field = core_error::Field;

    GlobalQFOTransferBarrierMap<QFOImageTransferBarrier> qfo_release_image_barrier_map;
    GlobalQFOTransferBarrierMap<QFOBufferTransferBarrier> qfo_release_buffer_barrier_map;
    VkValidationCacheEXT core_validation_cache = VK_NULL_HANDLE;
    std::string validation_cache_path;

    CoreChecks() { container_type = LayerObjectTypeCoreValidation; }

    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    struct SimpleErrorLocation {
        const char* func_name;
        const char* vuid;
        const char* FuncName() const { return func_name; }
        const std::string Vuid() const { return vuid; }
        SimpleErrorLocation(const char* func_name_, const char* vuid_) : func_name(func_name_), vuid(vuid_) {}
    };

    bool ValidateSetMemBinding(VkDeviceMemory mem, const BINDABLE& mem_binding, const char* apiName) const;
    bool ValidateDeviceQueueFamily(uint32_t queue_family, const char* cmd_name, const char* parameter_name, const char* error_code,
                                   bool optional) const;
    bool ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset, const void* pNext,
                                  const char* api_name) const;
    bool ValidateGetImageMemoryRequirements2(const VkImageMemoryRequirementsInfo2* pInfo, const char* func_name) const;
    bool CheckCommandBuffersInFlight(const COMMAND_POOL_STATE* pPool, const char* action, const char* error_code) const;
    bool CheckCommandBufferInFlight(const CMD_BUFFER_STATE* cb_state, const char* action, const char* error_code) const;
    void StoreMemRanges(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size);
    bool ValidateIdleDescriptorSet(VkDescriptorSet set, const char* func_str) const;
    bool ValidatePipelineLibraryFlags(const VkGraphicsPipelineLibraryFlagsEXT lib_flags,
                                      const VkPipelineLibraryCreateInfoKHR& link_info,
                                      const VkPipelineRenderingCreateInfo* rendering_struct, uint32_t pipe_index, int lib_index,
                                      const char* vuid) const;
    bool ValidatePipeline(std::vector<std::shared_ptr<PIPELINE_STATE>> const& pipelines, uint32_t pipe_index) const;
    bool ValidImageBufferQueue(const CMD_BUFFER_STATE& cb_state, const VulkanTypedHandle& object, uint32_t queueFamilyIndex,
                               uint32_t count, const uint32_t* indices) const;
    bool ValidateFenceForSubmit(const FENCE_STATE* pFence, const char* inflight_vuid, const char* retired_vuid,
                                const char* func_name) const;
    bool ValidateSemaphoresForSubmit(struct SemaphoreSubmitState& state, const VkSubmitInfo& submit, const Location& loc) const;
    bool ValidateSemaphoresForSubmit(struct SemaphoreSubmitState& state, const VkSubmitInfo2KHR& submit, const Location& loc) const;
    bool ValidateSemaphoresForSubmit(struct SemaphoreSubmitState& state, const VkBindSparseInfo& submit, const Location& loc) const;
    bool ValidateCBDynamicStatus(const CMD_BUFFER_STATE& cb_state, CBDynamicStatus status, CMD_TYPE cmd_type,
                                 const char* msg_code) const;
    bool ValidateDrawDynamicState(const CMD_BUFFER_STATE& cb_state, const PIPELINE_STATE& pipeline, CMD_TYPE cmd_type) const;
    bool LogInvalidAttachmentMessage(const char* type1_string, const RENDER_PASS_STATE& rp1_state, const char* type2_string,
                                     const RENDER_PASS_STATE& rp2_state, uint32_t primary_attach, uint32_t secondary_attach,
                                     const char* msg, const char* caller, const char* error_code) const;
    bool LogInvalidPnextMessage(const char* type1_string, const RENDER_PASS_STATE& rp1_state, const char* type2_string,
                                const RENDER_PASS_STATE& rp2_state, const char* msg, const char* caller,
                                const char* error_code) const;
    bool LogInvalidDependencyMessage(const char* type1_string, const RENDER_PASS_STATE& rp1_state, const char* type2_string,
                                     const RENDER_PASS_STATE& rp2_state, const char* msg, const char* caller,
                                     const char* error_code) const;
    bool ValidateStageMaskHost(const Location& loc, VkPipelineStageFlags2KHR stageMask) const;
    bool ValidateMapMemRange(const DEVICE_MEMORY_STATE* mem_info, VkDeviceSize offset, VkDeviceSize size) const;
    bool ValidateRenderPassDAG(RenderPassCreateVersion rp_version, const VkRenderPassCreateInfo2* pCreateInfo) const;
    bool ValidateAttachmentCompatibility(const char* type1_string, const RENDER_PASS_STATE& rp1_state, const char* type2_string,
                                         const RENDER_PASS_STATE& rp2_state, uint32_t primary_attach, uint32_t secondary_attach,
                                         const char* caller, const char* error_code) const;
    bool ValidateSubpassCompatibility(const char* type1_string, const RENDER_PASS_STATE& rp1_state, const char* type2_string,
                                      const RENDER_PASS_STATE& rp2_state, const int subpass, const char* caller,
                                      const char* error_code) const;
    bool ValidateDependencyCompatibility(const char* type1_string, const RENDER_PASS_STATE& rp1_state, const char* type2_string,
                                         const RENDER_PASS_STATE& rp2_state, const uint32_t dependency, const char* caller,
                                         const char* error_code) const;
    bool ValidateRenderPassCompatibility(const char* type1_string, const RENDER_PASS_STATE& rp1_state, const char* type2_string,
                                         const RENDER_PASS_STATE& rp2_state, const char* caller, const char* error_code) const;
    bool ReportInvalidCommandBuffer(const CMD_BUFFER_STATE& cb_state, const char* call_source) const;
    bool ValidateQueueFamilyIndex(const PHYSICAL_DEVICE_STATE* pd_state, uint32_t requested_queue_family, const char* err_code,
                                  const char* cmd_name, const char* queue_family_var_name) const;
    bool ValidateDeviceQueueCreateInfos(const PHYSICAL_DEVICE_STATE* pd_state, uint32_t info_count,
                                        const VkDeviceQueueCreateInfo* infos) const;

    bool ValidateProtectedImage(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& image_state, const char* cmd_name,
                                const char* vuid, const char* more_message = "") const;
    bool ValidateUnprotectedImage(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& image_state, const char* cmd_name,
                                  const char* vuid, const char* more_message = "") const;
    bool ValidateProtectedBuffer(const CMD_BUFFER_STATE& cb_state, const BUFFER_STATE& buffer_state, const char* cmd_name,
                                 const char* vuid, const char* more_message = "") const;
    bool ValidateUnprotectedBuffer(const CMD_BUFFER_STATE& cb_state, const BUFFER_STATE& buffer_state, const char* cmd_name,
                                   const char* vuid, const char* more_message = "") const;

    bool ValidatePipelineVertexDivisors(const safe_VkPipelineVertexInputStateCreateInfo& input_state,
                                        const std::vector<VkVertexInputBindingDescription>& binding_descriptions,
                                        const uint32_t pipe_index) const;
    bool ValidatePipelineCacheControlFlags(VkPipelineCreateFlags flags, uint32_t index, const char* caller_name,
                                           const char* vuid) const;
    bool ValidatePipelineProtectedAccessFlags(VkPipelineCreateFlags flags, uint32_t index) const;
    template <typename ImgBarrier>
    void EnqueueSubmitTimeValidateImageBarrierAttachment(const Location& loc, CMD_BUFFER_STATE* cb_state,
                                                         const ImgBarrier& barrier);
    template <typename ImgBarrier>
    bool ValidateImageBarrierAttachment(const Location& loc, CMD_BUFFER_STATE const* cb_state, const FRAMEBUFFER_STATE* framebuffer,
                                        uint32_t active_subpass, const safe_VkSubpassDescription2& sub_desc,
                                        const VkRenderPass rp_handle, const ImgBarrier& img_barrier,
                                        const CMD_BUFFER_STATE* primary_cb_state = nullptr) const;

    static bool ValidateConcurrentBarrierAtSubmit(const Location& loc, const ValidationStateTracker& state_data,
                                                  const QUEUE_STATE& queue_data, const CMD_BUFFER_STATE& cb_state,
                                                  const VulkanTypedHandle& typed_handle, uint32_t src_queue_family,
                                                  uint32_t dst_queue_family);
    bool ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, RenderPassCreateVersion rp_version,
                                    const VkRenderPassBeginInfo* pRenderPassBegin, CMD_TYPE cmd_type) const;
    bool ValidateDependencies(FRAMEBUFFER_STATE const* framebuffer, RENDER_PASS_STATE const* renderPass) const;
    template <typename Barrier>
    bool ValidateBufferBarrier(const LogObjectList& objlist, const Location& loc, const CMD_BUFFER_STATE* cb_state,
                               const Barrier& barrier) const;

    template <typename Barrier>
    bool ValidateImageBarrier(const LogObjectList& objlist, const Location& loc, const CMD_BUFFER_STATE* cb_state,
                              const Barrier& barrier) const;

    bool ValidateBarriers(const Location& loc, const CMD_BUFFER_STATE* cb_state, VkPipelineStageFlags src_stage_mask,
                          VkPipelineStageFlags dst_stage_mask, uint32_t memBarrierCount, const VkMemoryBarrier* pMemBarriers,
                          uint32_t bufferBarrierCount, const VkBufferMemoryBarrier* pBufferMemBarriers,
                          uint32_t imageMemBarrierCount, const VkImageMemoryBarrier* pImageMemBarriers) const;

    bool ValidatePipelineStageFeatureEnables(const LogObjectList& objlist, const Location& loc,
                                             VkPipelineStageFlags2KHR stage_mask) const;
    bool ValidatePipelineStage(const LogObjectList& objlist, const Location& loc, VkQueueFlags queue_flags,
                               VkPipelineStageFlags2KHR stage_mask) const;
    bool ValidateAccessMask(const LogObjectList& objlist, const Location& loc, VkQueueFlags queue_flags,
                            VkAccessFlags2KHR access_mask, VkPipelineStageFlags2KHR stage_mask) const;
    template <typename Barrier>
    bool ValidateMemoryBarrier(const LogObjectList& objlist, const Location& loc, const CMD_BUFFER_STATE* cb_state,
                               const Barrier& barrier, VkPipelineStageFlags src_stage_mask,
                               VkPipelineStageFlags dst_stage_mask) const;
    template <typename Barrier>
    bool ValidateMemoryBarrier(const LogObjectList& objlist, const Location& loc, const CMD_BUFFER_STATE* cb_state,
                               const Barrier& barrier) const;

    bool ValidateSubpassDependency(const LogObjectList& objlist, const Location& loc, const VkSubpassDependency2& barrier) const;

    bool ValidateDependencyInfo(const LogObjectList& objlist, const Location& loc, const CMD_BUFFER_STATE* cb_state,
                                const VkDependencyInfoKHR* dep_info) const;
    template <typename ImgBarrier>
    bool ValidateBarrierQueueFamilies(const Location& loc, const CMD_BUFFER_STATE* cb_state, const ImgBarrier& barrier,
                                      const IMAGE_STATE* state_data) const;
    template <typename BufBarrier>
    bool ValidateBarrierQueueFamilies(const Location& loc, const CMD_BUFFER_STATE* cb_state, const BufBarrier& barrier,
                                      const BUFFER_STATE* state_data) const;
    bool ValidateSwapchainPresentModesCreateInfo(VkPresentModeKHR present_mode, const char* func_name,
                                                 VkSwapchainCreateInfoKHR const* create_info,
                                                 const SURFACE_STATE* surface_state) const;
    bool ValidateSwapchainPresentScalingCreateInfo(VkPresentModeKHR present_mode, const char* func_name,
                                                   VkSurfaceCapabilitiesKHR* capabilities,
                                                   VkSwapchainCreateInfoKHR const* create_info,
                                                   const SURFACE_STATE* surface_state) const;
    bool ValidateCreateSwapchain(const char* func_name, VkSwapchainCreateInfoKHR const* pCreateInfo,
                                 const SURFACE_STATE* surface_state, const SWAPCHAIN_NODE* old_swapchain_state) const;
    bool ValidateGraphicsPipelineBindPoint(const CMD_BUFFER_STATE* cb_state, const PIPELINE_STATE& pipeline) const;
    bool ValidatePipelineBindPoint(const CMD_BUFFER_STATE* cb_state, VkPipelineBindPoint bind_point, const char* func_name,
                                   const std::map<VkPipelineBindPoint, std::string>& bind_errors) const;
    bool ValidateMemoryIsMapped(const char* funcName, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges) const;
    bool ValidateMappedMemoryRangeDeviceLimits(const char* func_name, uint32_t mem_range_count,
                                               const VkMappedMemoryRange* mem_ranges) const;
    bool ValidateSecondaryCommandBufferState(const CMD_BUFFER_STATE& cb_state, const CMD_BUFFER_STATE& sub_cb_state) const;
    bool ValidateInheritanceInfoFramebuffer(VkCommandBuffer primaryBuffer, const CMD_BUFFER_STATE& cb_state,
                                            VkCommandBuffer secondaryBuffer, const CMD_BUFFER_STATE& sub_cb_state,
                                            const char* caller) const;
    bool ValidateDescriptorUpdateTemplate(const char* func_name, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo) const;
    bool ValidateCreateSamplerYcbcrConversion(const char* func_name, const VkSamplerYcbcrConversionCreateInfo* create_info) const;
    bool ValidateImportFence(VkFence fence, const char* vuid, const char* caller_name) const;
    bool ValidateAcquireNextImage(VkDevice device, AcquireVersion version, VkSwapchainKHR swapchain, uint64_t timeout,
                                  VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, const char* func_name,
                                  const char* semaphore_type_vuid) const;
    bool VerifyRenderAreaBounds(const VkRenderPassBeginInfo* pRenderPassBegin, const char* func_name) const;
    bool VerifyFramebufferAndRenderPassImageViews(const VkRenderPassBeginInfo* pRenderPassBeginInfo, const char* func_name) const;
    bool ValidatePrimaryCommandBuffer(const CMD_BUFFER_STATE& cb_state, char const* cmd_name, const char* error_code) const;

    void RecordCmdNextSubpassLayouts(VkCommandBuffer commandBuffer, VkSubpassContents contents);
    bool ValidateCmdEndRenderPass(RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer, CMD_TYPE cmd_type,
                                  const VkSubpassEndInfo* pSubpassEndInfo) const;
    void RecordCmdEndRenderPassLayouts(VkCommandBuffer commandBuffer);
    bool MatchUsage(uint32_t count, const VkAttachmentReference2* attachments, const VkFramebufferCreateInfo* fbci,
                    VkImageUsageFlagBits usage_flag, const char* error_code) const;
    bool CheckDependencyExists(const VkRenderPass renderpass, const uint32_t subpass, const VkImageLayout layout,
                               const std::vector<SubpassLayout>& dependent_subpasses, const std::vector<DAGNode>& subpass_to_node,
                               bool& skip) const;
    bool CheckPreserved(const VkRenderPass renderpass, const VkRenderPassCreateInfo2* pCreateInfo, const int index,
                        const uint32_t attachment, const std::vector<DAGNode>& subpass_to_node, int depth, bool& skip) const;
    bool ValidateBindImageMemory(uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos, const char* api_name) const;
    bool ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                 const char* api_name) const;
    static bool ValidateCopyQueryPoolResults(const CMD_BUFFER_STATE& cb_state, VkQueryPool queryPool, uint32_t firstQuery,
                                             uint32_t queryCount, uint32_t perfPass, VkQueryResultFlags flags,
                                             QueryMap* localQueryToStateMap);
    static bool VerifyQueryIsReset(const CMD_BUFFER_STATE& cb_state, const QueryObject& query_obj, const CMD_TYPE cmd_type,
                                   VkQueryPool& firstPerfQueryPool, uint32_t perfPass, QueryMap* localQueryToStateMap);
    static bool ValidatePerformanceQuery(const CMD_BUFFER_STATE& cb_state, const QueryObject& query_obj, const CMD_TYPE cmd_type,
                                         VkQueryPool& firstPerfQueryPool, uint32_t perfPass, QueryMap* localQueryToStateMap);
    bool ValidateBeginQuery(const CMD_BUFFER_STATE& cb_state, const QueryObject& query_obj, VkFlags flags, uint32_t index,
                            CMD_TYPE cmd, const ValidateBeginQueryVuids* vuids) const;
    bool ValidateCmdEndQuery(const CMD_BUFFER_STATE& cb_state, const QueryObject& query_obj, uint32_t index, CMD_TYPE cmd,
                             const ValidateEndQueryVuids* vuids) const;

    const DrawDispatchVuid& GetDrawDispatchVuid(CMD_TYPE cmd_type) const;
    bool ValidateCmdDrawInstance(const CMD_BUFFER_STATE& cb_state, uint32_t instanceCount, uint32_t firstInstance,
                                 CMD_TYPE cmd_type) const;
    bool ValidateCmdDrawType(const CMD_BUFFER_STATE& cb_state, bool indexed, VkPipelineBindPoint bind_point,
                             CMD_TYPE cmd_type) const;
    bool ValidateCmdNextSubpass(RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer, CMD_TYPE cmd_type) const;
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
    bool ValidateCommandBufferState(const CMD_BUFFER_STATE& cb_state, const char* call_source, int current_submit_count,
                                    const char* vu_id) const;
    bool ValidateCommandBufferSimultaneousUse(const Location& loc, const CMD_BUFFER_STATE& cb_state,
                                              int current_submit_count) const;
    bool ValidateAttachmentReference(RenderPassCreateVersion rp_version, VkAttachmentReference2 reference,
                                     const VkFormat attachment_format, bool input, const char* error_type,
                                     const char* function_name) const;
    bool ValidateRenderpassAttachmentUsage(RenderPassCreateVersion rp_version, const VkRenderPassCreateInfo2* pCreateInfo,
                                           const char* function_name) const;
    bool AddAttachmentUse(RenderPassCreateVersion rp_version, uint32_t subpass, std::vector<uint8_t>& attachment_uses,
                          std::vector<VkImageLayout>& attachment_layouts, uint32_t attachment, uint8_t new_use,
                          VkImageLayout new_layout) const;
    bool ValidateAttachmentIndex(RenderPassCreateVersion rp_version, uint32_t attachment, uint32_t attachment_count,
                                 const char* error_type, const char* function_name) const;
    bool ValidateMSRTSSAttachmentSampleCount(VkDevice device, const char* function_name, uint32_t attachment_index,
                                             VkSampleCountFlagBits attachment_bits, VkSampleCountFlagBits msrtss_bit) const;
    bool ValidateCreateRenderPass(VkDevice device, RenderPassCreateVersion rp_version, const VkRenderPassCreateInfo2* pCreateInfo,
                                  const char* function_name) const;

    bool ValidateRenderPassPipelineBarriers(const Location& loc, const CMD_BUFFER_STATE* cb_state,
                                            VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                            VkDependencyFlags dependency_flags, uint32_t mem_barrier_count,
                                            const VkMemoryBarrier* mem_barriers, uint32_t buffer_mem_barrier_count,
                                            const VkBufferMemoryBarrier* buffer_mem_barriers, uint32_t image_mem_barrier_count,
                                            const VkImageMemoryBarrier* image_barriers) const;
    bool ValidateRenderPassPipelineBarriers(const Location& loc, const CMD_BUFFER_STATE* cb_state,
                                            const VkDependencyInfoKHR* dep_info) const;

    bool ValidateStageMasksAgainstQueueCapabilities(const LogObjectList& objlist, const Location& loc, VkQueueFlags queue_flags,
                                                    VkPipelineStageFlags2KHR stage_mask) const;
    bool ValidateUpdateDescriptorSetWithTemplate(VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                 const void* pData) const;
    template <typename HandleT>
    bool ValidateMemoryIsBoundToBuffer(HandleT handle, const BUFFER_STATE& buffer_state, const char* api_name,
                                       const char* error_code) const {
        bool result = false;
        if (!buffer_state.sparse) {
            const LogObjectList objlist(handle, buffer_state.Handle());
            result |= VerifyBoundMemoryIsValid(buffer_state.MemState(), objlist, buffer_state.Handle(), api_name, error_code);
        }
        return result;
    }

    bool ValidateHostVisibleMemoryIsBoundToBuffer(const BUFFER_STATE&, const char*, const char*) const;

    template <typename HandleT>
    bool ValidateMemoryIsBoundToImage(HandleT handle, const IMAGE_STATE& image_state, const char* api_name,
                                      const char* error_code) const {
        return ValidateMemoryIsBoundToImage<HandleT, SimpleErrorLocation>(handle, image_state,
                                                                          SimpleErrorLocation(api_name, error_code));
    }
    template <typename HandleT>
    bool ValidateMemoryIsBoundToImage(HandleT handle, const IMAGE_STATE& image_state, const Location& loc) const {
        using LocationAdapter = core_error::LocationVuidAdapter<sync_vuid_maps::GetImageBarrierVUIDFunctor>;
        return ValidateMemoryIsBoundToImage<HandleT, LocationAdapter>(handle, image_state,
                                                                      LocationAdapter(loc, sync_vuid_maps::ImageError::kNoMemory));
    }
    template <typename HandleT, typename Location>
    bool ValidateMemoryIsBoundToImage(HandleT handle, const IMAGE_STATE&, const Location&) const;

    template <typename HandleT>
    bool ValidateMemoryIsBoundToAccelerationStructure(HandleT handle, const ACCELERATION_STRUCTURE_STATE& as_state,
                                                      const char* api_name, const char* error_code) const {
        const LogObjectList objlist(handle, as_state.Handle());
        return VerifyBoundMemoryIsValid(as_state.MemState(), objlist, as_state.Handle(), api_name, error_code);
    }
    template <typename HandleT>
    bool ValidateMemoryIsBoundToAccelerationStructure(HandleT handle, const ACCELERATION_STRUCTURE_STATE_KHR& as_state,
                                                      const char* api_name, const char* error_code) const {
        const LogObjectList objlist(handle, as_state.Handle());
        return VerifyBoundMemoryIsValid(as_state.buffer_state->MemState(), objlist, as_state.Handle(), api_name, error_code);
    }
    bool ValidateObjectNotInUse(const BASE_NODE* obj_node, const char* caller_name, const char* error_code) const;
    bool ValidateCmdQueueFlags(const CMD_BUFFER_STATE& cb_state, const char* caller_name, VkQueueFlags flags,
                               const char* error_code) const;
    bool ValidateSampleLocationsInfo(const VkSampleLocationsInfoEXT* pSampleLocationsInfo, const char* apiName) const;
    bool MatchSampleLocationsInfo(const VkSampleLocationsInfoEXT* pSampleLocationsInfo1,
                                  const VkSampleLocationsInfoEXT* pSampleLocationsInfo2) const;
    bool InsideRenderPass(const CMD_BUFFER_STATE& cb_state, const char* apiName, const char* msgCode) const;
    bool OutsideRenderPass(const CMD_BUFFER_STATE& cb_state, const char* apiName, const char* msgCode) const;
    bool InsideVideoCodingScope(const CMD_BUFFER_STATE& cb_state, const char* apiName, const char* msgCode) const;
    bool OutsideVideoCodingScope(const CMD_BUFFER_STATE& cb_state, const char* apiName, const char* msgCode) const;
    std::vector<VkVideoFormatPropertiesKHR> GetVideoFormatProperties(VkImageUsageFlags image_usage,
                                                                     const VkVideoProfileListInfoKHR* profile_list) const;
    std::vector<VkVideoFormatPropertiesKHR> GetVideoFormatProperties(VkImageUsageFlags image_usage,
                                                                     const VkVideoProfileInfoKHR* profile) const;
    bool IsVideoFormatSupported(VkFormat format, VkImageUsageFlags image_usage, const VkVideoProfileInfoKHR* profile) const;
    bool ValidateVideoPictureResource(const VideoPictureResource& picture_resource, VkCommandBuffer cmdbuf,
                                      const VIDEO_SESSION_STATE& vs_state, const char* api_name, const char* where,
                                      const char* coded_offset_vuid = nullptr, const char* coded_extent_vuid = nullptr) const;
    template <typename T1>
    bool ValidateVideoProfileInfo(const VkVideoProfileInfoKHR* profile, const T1 object, const char* api_name,
                                  const char* where) const;
    template <typename T1>
    bool ValidateVideoProfileListInfo(const VkVideoProfileListInfoKHR* profile_list, const T1 object, const char* api_name,
                                      bool expect_decode_profile, const char* missing_decode_profile_msg_code,
                                      bool expect_encode_profile, const char* missing_encode_profile_msg_code) const;
    bool ValidateDecodeH264ParametersAddInfo(const VkVideoDecodeH264SessionParametersAddInfoKHR* add_info, VkDevice device,
                                             const char* api_name, const char* where,
                                             const VkVideoDecodeH264SessionParametersCreateInfoKHR* create_info = nullptr,
                                             const VIDEO_SESSION_PARAMETERS_STATE* template_state = nullptr) const;
    bool ValidateDecodeH265ParametersAddInfo(const VkVideoDecodeH265SessionParametersAddInfoKHR* add_info, VkDevice device,
                                             const char* api_name, const char* where,
                                             const VkVideoDecodeH265SessionParametersCreateInfoKHR* create_info = nullptr,
                                             const VIDEO_SESSION_PARAMETERS_STATE* template_state = nullptr) const;
    bool ValidateVideoDecodeInfoH264(const CMD_BUFFER_STATE& cb_state, const VkVideoDecodeInfoKHR& decode_info) const;
    bool ValidateVideoDecodeInfoH265(const CMD_BUFFER_STATE& cb_state, const VkVideoDecodeInfoKHR& decode_info) const;
    bool ValidateActiveReferencePictureCount(const CMD_BUFFER_STATE& cb_state, const VkVideoDecodeInfoKHR& decode_info) const;
    bool ValidateReferencePictureUseCount(const CMD_BUFFER_STATE& cb_state, const VkVideoDecodeInfoKHR& decode_info) const;
    bool ValidateImageSampleCount(VkCommandBuffer, const IMAGE_STATE& image_state, VkSampleCountFlagBits sample_count,
                                  const char* location, const std::string& msgCode) const;
    bool ValidateCmdSubpassState(const CMD_BUFFER_STATE& cb_state, const CMD_TYPE cmd_type) const;
    bool ValidateCmd(const CMD_BUFFER_STATE& cb_state, const CMD_TYPE cmd) const;
    bool ValidateIndirectCmd(const CMD_BUFFER_STATE& cb_state, const BUFFER_STATE& buffer_state, CMD_TYPE cmd_type) const;
    bool ValidateIndirectCountCmd(const CMD_BUFFER_STATE& cb_state, const BUFFER_STATE& count_buffer_state,
                                  VkDeviceSize count_buffer_offset, CMD_TYPE cmd_type) const;
    bool ValidateMultisampledRenderToSingleSampleView(VkCommandBuffer commandBuffer,
                                                      const std::shared_ptr<const IMAGE_VIEW_STATE>& image_view_state,
                                                      const VkMultisampledRenderToSingleSampledInfoEXT* msrtss_info,
                                                      const char* attachment_type, const char* func_name) const;

    bool ValidateDeviceMaskToPhysicalDeviceCount(uint32_t deviceMask, const LogObjectList& objlist, const char* VUID) const;
    bool ValidateDeviceMaskToZero(uint32_t deviceMask, const LogObjectList& objlist, const char* VUID) const;
    bool ValidateDeviceMaskToCommandBuffer(const CMD_BUFFER_STATE& cb_state, uint32_t deviceMask, const LogObjectList& objlist,
                                           const char* VUID) const;
    bool ValidateDeviceMaskToRenderPass(const CMD_BUFFER_STATE& cb_state, uint32_t deviceMask, const char* VUID) const;

    bool ValidateDepthStencilResolve(const VkRenderPassCreateInfo2* pCreateInfo, const char* function_name) const;

    bool ValidateBindAccelerationStructureMemory(VkDevice device, const VkBindAccelerationStructureMemoryInfoNV& info) const;
    // Prototypes for CoreChecks accessor functions
    VkFormatProperties3KHR GetPDFormatProperties(const VkFormat format) const;
    const VkPhysicalDeviceMemoryProperties* GetPhysicalDeviceMemoryProperties();

    bool FormatRequiresYcbcrConversionExplicitly(const VkFormat format) const;

    template <typename TransferBarrier>
    bool ValidateQueuedQFOTransferBarriers(const CMD_BUFFER_STATE& cb_state, QFOTransferCBScoreboards<TransferBarrier>* scoreboards,
                                           const GlobalQFOTransferBarrierMap<TransferBarrier>& global_release_barriers) const;
    bool ValidateQueuedQFOTransfers(const CMD_BUFFER_STATE& cb_state,
                                    QFOTransferCBScoreboards<QFOImageTransferBarrier>* qfo_image_scoreboards,
                                    QFOTransferCBScoreboards<QFOBufferTransferBarrier>* qfo_buffer_scoreboards) const;

    template <typename Barrier, typename Scoreboard>
    bool ValidateAndUpdateQFOScoreboard(const debug_report_data* report_data, const CMD_BUFFER_STATE& cb_state,
                                        const char* operation, const Barrier& barrier, Scoreboard* scoreboard) const;
    template <typename Barrier, typename TransferBarrier>
    void RecordBarrierValidationInfo(const Location& loc, CMD_BUFFER_STATE* cb_state, const Barrier& barrier,
                                     QFOTransferBarrierSets<TransferBarrier>& barrier_sets);

    template <typename Barrier, typename TransferBarrier>
    bool ValidateQFOTransferBarrierUniqueness(const Location& loc, const CMD_BUFFER_STATE* cb_state, const Barrier& barrier,
                                              const QFOTransferBarrierSets<TransferBarrier>& barrier_sets) const;

    bool ValidatePrimaryCommandBufferState(const Location& loc, const CMD_BUFFER_STATE& cb_state, int current_submit_count,
                                           QFOTransferCBScoreboards<QFOImageTransferBarrier>* qfo_image_scoreboards,
                                           QFOTransferCBScoreboards<QFOBufferTransferBarrier>* qfo_buffer_scoreboards) const;
    bool ValidatePipelineDrawtimeState(const LAST_BOUND_STATE& state, const CMD_BUFFER_STATE& cb_state, CMD_TYPE cmd_type,
                                       const PIPELINE_STATE& pipeline) const;
    bool ValidateCmdBufDrawState(const CMD_BUFFER_STATE& cb_state, CMD_TYPE cmd_type, const bool indexed,
                                 const VkPipelineBindPoint bind_point) const;
    bool ValidateCmdRayQueryState(const CMD_BUFFER_STATE& cb_state, CMD_TYPE cmd_type, const VkPipelineBindPoint bind_point) const;
    static bool ValidateEventStageMask(const CMD_BUFFER_STATE& cb_state, size_t eventCount, size_t firstEventIndex,
                                       VkPipelineStageFlags2KHR sourceStageMask, EventToStageMap* localEventToStageMap);
    bool ValidateQueueFamilyIndices(const Location& loc, const CMD_BUFFER_STATE& cb_state, VkQueue queue) const;
    VkResult CoreLayerCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator,
                                               VkValidationCacheEXT* pValidationCache) override;
    void CoreLayerDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                            const VkAllocationCallbacks* pAllocator) override;
    VkResult CoreLayerMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                               const VkValidationCacheEXT* pSrcCaches) override;
    VkResult CoreLayerGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                void* pData) override;
    // For given bindings validate state at time of draw is correct, returning false on error and writing error details into string*
    bool ValidateDrawState(const cvdescriptorset::DescriptorSet& descriptor_set, const BindingReqMap& bindings,
                           const std::vector<uint32_t>& dynamic_offsets, const CMD_BUFFER_STATE& cb_state, const char* caller,
                           const DrawDispatchVuid& vuids) const;

    bool VerifySetLayoutCompatibility(const cvdescriptorset::DescriptorSetLayout& layout_dsl,
                                      const cvdescriptorset::DescriptorSetLayout& bound_dsl, std::string& error_msg) const;

    bool VerifySetLayoutCompatibility(const cvdescriptorset::DescriptorSet& descriptor_set,
                                      const PIPELINE_LAYOUT_STATE& pipeline_layout, const uint32_t layoutIndex,
                                      std::string& errorMsg) const;

    struct DescriptorContext {
        const char* caller;
        const DrawDispatchVuid& vuids;
        const CMD_BUFFER_STATE& cb_state;
        const cvdescriptorset::DescriptorSet& descriptor_set;
        const VkFramebuffer framebuffer;
        bool record_time_validate;
        const std::vector<uint32_t>& dynamic_offsets;
        std::optional<vvl::unordered_map<VkImageView, VkImageLayout>>& checked_layouts;
    };
    using DescriptorBindingInfo = std::pair<const uint32_t, DescriptorRequirement>;

    bool ValidateDescriptorSetBindingData(const DescriptorContext& context, const DescriptorBindingInfo& binding_info,
                                          const cvdescriptorset::DescriptorBinding& binding) const;

    template <typename T>
    bool ValidateDescriptors(const DescriptorContext& context, const DescriptorBindingInfo& binding_info, const T& binding) const;

    bool ValidateDescriptor(const DescriptorContext& context, const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type, const cvdescriptorset::BufferDescriptor& descriptor) const;
    bool ValidateDescriptor(const DescriptorContext& context, const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type, const cvdescriptorset::ImageDescriptor& descriptor) const;
    bool ValidateDescriptor(const DescriptorContext& context, const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type, const cvdescriptorset::ImageSamplerDescriptor& descriptor) const;
    bool ValidateDescriptor(const DescriptorContext& context, const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type, const cvdescriptorset::TexelDescriptor& descriptor) const;
    bool ValidateDescriptor(const DescriptorContext& context, const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type,
                            const cvdescriptorset::AccelerationStructureDescriptor& descriptor) const;
    bool ValidateDescriptor(const DescriptorContext& context, const DescriptorBindingInfo& binding_info, uint32_t index,
                            VkDescriptorType descriptor_type, const cvdescriptorset::SamplerDescriptor& descriptor) const;

    // helper for the common parts of ImageSamplerDescriptor and SamplerDescriptor validation
    bool ValidateSamplerDescriptor(const DescriptorContext& context, const cvdescriptorset::DescriptorSet& descriptor_set,
                                   const DescriptorBindingInfo& binding_info, uint32_t index, VkSampler sampler, bool is_immutable,
                                   const SAMPLER_STATE* sampler_state) const;

    // Validate contents of a CopyUpdate
    using DescriptorSet = cvdescriptorset::DescriptorSet;
    bool ValidateCopyUpdate(const VkCopyDescriptorSet* update, const DescriptorSet* dst_set, const DescriptorSet* src_set,
                            const char* func_name, std::string* error_code, std::string* error_msg) const;
    bool VerifyCopyUpdateContents(const VkCopyDescriptorSet* update, const DescriptorSet* src_set, VkDescriptorType src_type,
                                  uint32_t src_index, const DescriptorSet* dst_set, VkDescriptorType dst_type, uint32_t dst_index,
                                  const char* func_name, std::string* error_code, std::string* error_msg) const;
    // Validate contents of a WriteUpdate
    bool ValidateWriteUpdate(const DescriptorSet* descriptor_set, const VkWriteDescriptorSet* update, const char* func_name,
                             std::string* error_code, std::string* error_msg, bool push) const;
    bool VerifyWriteUpdateContents(const DescriptorSet* dest_set, const VkWriteDescriptorSet* update, const uint32_t index,
                                   const char* func_name, std::string* error_code, std::string* error_msg, bool push) const;
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
    template <typename T>
    bool ValidateAccelerationStructureUpdate(T acc, const char* func_name, std::string* error_code, std::string* error_msg) const;
    bool ValidateUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet, const UPDATE_TEMPLATE_STATE* template_state,
                                                     const void* pData) const;
    bool ValidateAllocateDescriptorSets(const VkDescriptorSetAllocateInfo*,
                                        const cvdescriptorset::AllocateDescriptorSetsData*) const;
    bool ValidateUpdateDescriptorSets(uint32_t write_count, const VkWriteDescriptorSet* p_wds, uint32_t copy_count,
                                      const VkCopyDescriptorSet* p_cds, const char* func_name) const;

    // Stuff from shader_validation
    bool ValidateGraphicsPipelineShaderState(const PIPELINE_STATE& pipeline) const;
    bool ValidateGraphicsPipelineLibrary(const PIPELINE_STATE& pipeline) const;
    bool ValidateGraphicsPipelineShaderDynamicState(const PIPELINE_STATE& pipeline, const CMD_BUFFER_STATE& cb_state,
                                                    const char* caller, const DrawDispatchVuid& vuid) const;
    bool ValidateGraphicsPipelineBlendEnable(const PIPELINE_STATE& pipeline) const;
    bool ValidateGraphicsPipelinePreRasterState(const PIPELINE_STATE& pipeline) const;
    bool ValidateGraphicsPipelineColorBlendState(const PIPELINE_STATE& pipeline,
                                                 const safe_VkSubpassDescription2* subpass_desc) const;
    bool ValidateGraphicsPipelineRasterizationState(const PIPELINE_STATE& pipeline,
                                                    const safe_VkSubpassDescription2* subpass_desc) const;
    bool ValidateGraphicsPipelineMultisampleState(const PIPELINE_STATE& pipeline,
                                                  const safe_VkSubpassDescription2* subpass_desc) const;
    bool ValidateGraphicsPipelineDynamicState(const PIPELINE_STATE& pipeline) const;
    bool ValidateGraphicsPipelineFragmentShadingRateState(const PIPELINE_STATE& pipeline) const;
    bool ValidateGraphicsPipelineDynamicRendering(const PIPELINE_STATE& pipeline) const;
    bool ValidateComputePipelineShaderState(const PIPELINE_STATE& pipeline) const;
    uint32_t CalcShaderStageCount(const PIPELINE_STATE& pipeline, VkShaderStageFlagBits stageBit) const;
    bool GroupHasValidIndex(const PIPELINE_STATE& pipeline, uint32_t group, uint32_t stage) const;
    bool ValidateRayTracingPipeline(const PIPELINE_STATE& pipeline, const safe_VkRayTracingPipelineCreateInfoCommon& create_info,
                                    VkPipelineCreateFlags flags, bool isKHR) const;
    bool PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                     VkShaderModuleIdentifierEXT* pIdentifier) const override;
    bool PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                               VkShaderModuleIdentifierEXT* pIdentifier) const override;
    bool PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) const override;
    bool ValidatePipelineShaderStage(const PIPELINE_STATE& pipeline, const PipelineStageState& stage_state) const;
    bool ValidatePointSizeShaderState(const PIPELINE_STATE& pipeline, const SHADER_MODULE_STATE& module_state,
                                      const Instruction& entrypoint, VkShaderStageFlagBits stage) const;
    bool ValidatePrimitiveRateShaderState(const PIPELINE_STATE& pipeline, const SHADER_MODULE_STATE& module_state,
                                          const Instruction& entrypoint, VkShaderStageFlagBits stage) const;
    bool ValidateTexelOffsetLimits(const SHADER_MODULE_STATE& module_state, const Instruction& insn) const;

    // Auto-generated helper functions
    bool ValidateShaderCapabilitiesAndExtensions(const Instruction& insn) const;
    VkFormat CompatibleSpirvImageFormat(uint32_t spirv_image_format) const;

    bool ValidateShaderStageWritableOrAtomicDescriptor(const SHADER_MODULE_STATE& module_state, VkShaderStageFlagBits stage,
                                                       bool has_writable_descriptor, bool has_atomic_descriptor) const;
    bool ValidateShaderStageInputOutputLimits(const SHADER_MODULE_STATE& module_state,
                                              safe_VkPipelineShaderStageCreateInfo const* pStage, const PIPELINE_STATE& pipeline,
                                              const Instruction& entrypoint) const;
    bool ValidateShaderStorageImageFormatsVariables(const SHADER_MODULE_STATE& module_state, const Instruction* insn) const;
    bool ValidateShaderStageMaxResources(const SHADER_MODULE_STATE& module_state, VkShaderStageFlagBits stage,
                                         const PIPELINE_STATE& pipeline) const;
    bool ValidateShaderStageGroupNonUniform(const SHADER_MODULE_STATE& module_state, VkShaderStageFlagBits stage,
                                            const Instruction& insn) const;
    bool ValidateMemoryScope(const SHADER_MODULE_STATE& module_state, const Instruction& insn) const;
    bool ValidateCooperativeMatrix(const SHADER_MODULE_STATE& module_state,
                                   safe_VkPipelineShaderStageCreateInfo const* pStage) const;
    bool ValidateShaderResolveQCOM(const SHADER_MODULE_STATE& module_state, safe_VkPipelineShaderStageCreateInfo const* pStage,
                                   const PIPELINE_STATE& pipeline) const;
    bool ValidateShaderSubgroupSizeControl(const SHADER_MODULE_STATE& module_state,
                                           safe_VkPipelineShaderStageCreateInfo const* pStage) const;
    bool ValidateComputeSharedMemory(const SHADER_MODULE_STATE& module_state, uint32_t total_shared_size) const;
    bool ValidateAtomicsTypes(const SHADER_MODULE_STATE& module_state) const;
    bool ValidateExecutionModes(const SHADER_MODULE_STATE& module_state, const Instruction& entrypoint, VkShaderStageFlagBits stage,
                                const PIPELINE_STATE& pipeline) const;
    bool ValidateViAgainstVsInputs(const PIPELINE_STATE& pipeline, const SHADER_MODULE_STATE& module_state,
                                   const Instruction& entrypoint) const;
    bool ValidateFsOutputsAgainstRenderPass(const SHADER_MODULE_STATE& module_state, const Instruction& entrypoint,
                                            const PIPELINE_STATE& pipeline, uint32_t subpass_index) const;
    bool ValidateFsOutputsAgainstDynamicRenderingRenderPass(const SHADER_MODULE_STATE& module_state, const Instruction& entrypoint,
                                                            const PIPELINE_STATE& pipeline) const;
    bool ValidateShaderInputAttachment(const SHADER_MODULE_STATE& module_state, const Instruction& entrypoint,
                                       const PIPELINE_STATE& pipeline) const;
    bool ValidatePushConstantUsage(const PIPELINE_STATE& pipeline, const SHADER_MODULE_STATE& module_state,
                                   safe_VkPipelineShaderStageCreateInfo const* pStage, const std::string& vuid) const;
    bool ValidateBuiltinLimits(const SHADER_MODULE_STATE& module_state, const Instruction& entrypoint,
                               const PIPELINE_STATE& pipeline) const;
    PushConstantByteState ValidatePushConstantSetUpdate(const std::vector<uint8_t>& push_constant_data_update,
                                                        const SHADER_MODULE_STATE::StructInfo& push_constant_used_in_shader,
                                                        uint32_t& out_issue_index) const;
    bool ValidateSpecializations(const SHADER_MODULE_STATE& module_state, safe_VkPipelineShaderStageCreateInfo const* info,
                                 const PIPELINE_STATE& pipeline) const;
    bool RequirePropertyFlag(const SHADER_MODULE_STATE& module_state, VkBool32 check, char const* flag, char const* structure,
                             const char* vuid) const;
    bool RequireFeature(const SHADER_MODULE_STATE& module_state, VkBool32 feature, char const* feature_name,
                        const char* vuid) const;
    bool ValidateInterfaceBetweenStages(const SHADER_MODULE_STATE& producer, const Instruction& producer_entrypoint,
                                        shader_stage_attributes const* producer_stage, const SHADER_MODULE_STATE& consumer,
                                        const Instruction& consumer_entrypoint, shader_stage_attributes const* consumer_stage,
                                        uint32_t pipe_index) const;
    bool ValidateDecorations(const SHADER_MODULE_STATE& module_state, const PIPELINE_STATE& pipeline) const;
    bool ValidateVariables(const SHADER_MODULE_STATE& module_state) const;
    bool ValidateTransformFeedback(const SHADER_MODULE_STATE& module_state, const PIPELINE_STATE& pipeline) const;
    bool ValidateShaderModuleId(const SHADER_MODULE_STATE& module_state, const PipelineStageState& stage_state,
                                const safe_VkPipelineShaderStageCreateInfo* pStage, const VkPipelineCreateFlags flags) const;
    bool ValidateShaderClock(const SHADER_MODULE_STATE& module_state, const Instruction& insn) const;
    bool ValidateImageWrite(const SHADER_MODULE_STATE& module_state, const Instruction& insn) const;

    template <typename RegionType>
    bool ValidateCopyImageTransferGranularityRequirements(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE* src_img,
                                                          const IMAGE_STATE* dst_img, const RegionType* region, const uint32_t i,
                                                          const char* function, CMD_TYPE cmd_type) const;
    bool ValidateUsageFlags(VkFlags actual, VkFlags desired, VkBool32 strict, const LogObjectList& objlist,
                            const VulkanTypedHandle& typed_handle, const char* msgCode, char const* func_name,
                            char const* usage_str) const;
    bool ValidateImageSubresourceRange(const uint32_t image_mip_count, const uint32_t image_layer_count,
                                       const VkImageSubresourceRange& subresourceRange, const char* cmd_name,
                                       const char* param_name, const char* image_layer_count_var_name, const VkImage image,
                                       const SubresourceRangeErrorCodes& errorCodes) const;
    bool ValidateMultipassRenderedToSingleSampledSampleCount(RenderPassCreateVersion rp_version, VkFramebuffer framebuffer,
                                                             VkRenderPass renderpass, uint32_t subpass, IMAGE_STATE* image_state,
                                                             VkSampleCountFlagBits msrtss_samples, uint32_t attachment_index,
                                                             bool depth = false) const;
    bool ValidateRenderPassLayoutAgainstFramebufferImageUsage(RenderPassCreateVersion rp_version, VkImageLayout layout,
                                                              const IMAGE_VIEW_STATE& image_view_state, VkFramebuffer framebuffer,
                                                              VkRenderPass renderpass, uint32_t attachment_index,
                                                              const char* variable_name) const;
    template <typename RegionType>
    bool ValidateBufferImageCopyData(const CMD_BUFFER_STATE& cb_state, uint32_t regionCount, const RegionType* pRegions,
                                     const IMAGE_STATE* image_state, const char* function, CMD_TYPE cmd_type,
                                     bool image_to_buffer) const;
    bool ValidateBufferViewRange(const BUFFER_STATE* buffer_state, const VkBufferViewCreateInfo* pCreateInfo,
                                 const VkPhysicalDeviceLimits* device_limits) const;
    bool ValidateBufferViewBuffer(const BUFFER_STATE* buffer_state, const VkBufferViewCreateInfo* pCreateInfo) const;

    bool ValidateImageFormatFeatures(const VkImageCreateInfo* pCreateInfo) const;

    bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkImage* pImage) const override;

    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, VkResult result) override;

    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) override;

    bool PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) const override;

    bool ValidateImageAttributes(const IMAGE_STATE* image_state, const VkImageSubresourceRange& range,
                                 const char* param_name) const;

    bool ValidateClearAttachmentExtent(const CMD_BUFFER_STATE& cb_state, uint32_t attachment_index,
                                       const IMAGE_VIEW_STATE* image_view_state, const VkRect2D& render_area, uint32_t rect_count,
                                       const VkClearRect* clear_rects) const;

    template <typename RegionType>
    bool ValidateImageCopyData(const uint32_t regionCount, const RegionType* pRegions, const IMAGE_STATE* src_state,
                               const IMAGE_STATE* dst_state, CMD_TYPE cmd_type) const;

    bool VerifyClearImageLayout(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE* image_state,
                                const VkImageSubresourceRange& range, VkImageLayout dest_image_layout, const char* func_name) const;

    template <typename RangeFactory>
    bool VerifyImageLayoutRange(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& image_state, VkImageAspectFlags aspect_mask,
                                VkImageLayout explicit_layout, const RangeFactory& range_factory, const char* caller,
                                const char* layout_mismatch_msg_code, bool* error) const;

    bool VerifyImageLayout(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& image_state, const VkImageSubresourceRange& range,
                           VkImageAspectFlags view_aspect, VkImageLayout explicit_layout, VkImageLayout optimal_layout,
                           const char* caller, const char* layout_invalid_msg_code, const char* layout_mismatch_msg_code,
                           bool* error) const;

    bool VerifyImageLayout(const CMD_BUFFER_STATE& cb_state, const IMAGE_VIEW_STATE& image_view_state,
                           VkImageLayout explicit_layout, const char* caller, const char* layout_mismatch_msg_code,
                           bool* error) const;

    bool VerifyImageLayout(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& image_state, const VkImageSubresourceRange& range,
                           VkImageLayout explicit_layout, VkImageLayout optimal_layout, const char* caller,
                           const char* layout_invalid_msg_code, const char* layout_mismatch_msg_code, bool* error) const {
        return VerifyImageLayout(cb_state, image_state, range, 0, explicit_layout, optimal_layout, caller, layout_invalid_msg_code,
                                 layout_mismatch_msg_code, error);
    }

    bool VerifyImageLayout(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& image_state,
                           const VkImageSubresourceLayers& subLayers, VkImageLayout explicit_layout, VkImageLayout optimal_layout,
                           const char* caller, const char* layout_invalid_msg_code, const char* layout_mismatch_msg_code,
                           bool* error) const;

    bool VerifyImageLayout(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& image_state, const VkImageSubresourceRange& range,
                           VkImageLayout explicit_layout, const char* caller, const char* layout_mismatch_msg_code,
                           bool* error) const;

    bool CheckItgExtent(const LogObjectList& objlist, const VkExtent3D& extent, const VkOffset3D& offset,
                        const VkExtent3D& granularity, const VkExtent3D& subresource_extent, const VkImageType image_type,
                        const uint32_t i, const char* function, const char* member, const char* vuid) const;

    bool CheckItgOffset(const LogObjectList& objlist, const VkOffset3D& offset, const VkExtent3D& granularity, const uint32_t i,
                        const char* function, const char* member, const char* vuid) const;
    VkExtent3D GetScaledItg(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE* img) const;

    bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                           const VkClearColorValue* pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange* pRanges) const override;

    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges) override;

    bool ValidateClearDepthStencilValue(VkCommandBuffer commandBuffer, VkClearDepthStencilValue clearValue,
                                        const char* apiName) const;

    bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                  const VkImageSubresourceRange* pRanges) const override;

    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges) override;

    bool FindLayouts(const IMAGE_STATE& image_state, std::vector<VkImageLayout>& layouts) const;

    bool VerifyFramebufferAndRenderPassLayouts(RenderPassCreateVersion rp_version, const CMD_BUFFER_STATE& cb_state,
                                               const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const FRAMEBUFFER_STATE* framebuffer_state) const;
    void RecordCmdBeginRenderPassLayouts(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         const VkSubpassContents contents);
    void TransitionAttachmentRefLayout(CMD_BUFFER_STATE* cb_state, FRAMEBUFFER_STATE* pFramebuffer,
                                       const safe_VkAttachmentReference2& ref);

    void TransitionSubpassLayouts(CMD_BUFFER_STATE* cb_state, const RENDER_PASS_STATE*, const int, FRAMEBUFFER_STATE*);

    void TransitionBeginRenderPassLayouts(CMD_BUFFER_STATE* cb_state, const RENDER_PASS_STATE*, FRAMEBUFFER_STATE*);

    template <typename ImageBarrier>
    bool UpdateCommandBufferImageLayoutMap(const CMD_BUFFER_STATE* cb_state, const Location& loc, const ImageBarrier& img_barrier,
                                           const CommandBufferImageLayoutMap& current_map,
                                           CommandBufferImageLayoutMap& layout_updates) const;

    bool ValidateBarrierLayoutToImageUsage(const Location& loc, VkImage image, VkImageLayout layout, VkImageUsageFlags usage) const;

    template <typename ImageBarrier>
    bool ValidateBarriersToImages(const Location& loc, const CMD_BUFFER_STATE* cb_state, uint32_t imageMemoryBarrierCount,
                                  const ImageBarrier* pImageMemoryBarriers) const;

    void RecordQueuedQFOTransfers(CMD_BUFFER_STATE* pCB);

    template <typename ImgBarrier>
    void TransitionImageLayouts(CMD_BUFFER_STATE* cb_state, uint32_t barrier_count, const ImgBarrier* barrier);

    template <typename ImgBarrier>
    void RecordTransitionImageLayout(CMD_BUFFER_STATE* cb_state, const IMAGE_STATE* image_state, const ImgBarrier& img_barrier,
                                     bool is_release_op);
    void RecordBarriers(Func func_name, CMD_BUFFER_STATE* cb_state, uint32_t bufferBarrierCount,
                        const VkBufferMemoryBarrier* pBufferMemBarriers, uint32_t imageMemBarrierCount,
                        const VkImageMemoryBarrier* pImageMemBarriers);
    void RecordBarriers(Func func_name, CMD_BUFFER_STATE* cb_state, const VkDependencyInfoKHR& dep_info);

    void TransitionFinalSubpassLayouts(CMD_BUFFER_STATE* cb_state, const VkRenderPassBeginInfo* pRenderPassBegin,
                                       FRAMEBUFFER_STATE* framebuffer_state);

    template <typename RegionType>
    bool ValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType* pRegions,
                              CMD_TYPE cmd_type) const;

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy* pRegions) const override;

    bool PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo) const override;

    bool PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) const override;

    bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment* pAttachments, uint32_t rectCount,
                                            const VkClearRect* pRects) const override;
    void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                          const VkClearAttachment* pAttachments, uint32_t rectCount,
                                          const VkClearRect* pRects) override;

    template <typename RegionType>
    bool ValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType* pRegions,
                                 CMD_TYPE cmd_type) const;

    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve* pRegions) const override;

    bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                            const VkResolveImageInfo2KHR* pResolveImageInfo) const override;

    bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer,
                                         const VkResolveImageInfo2* pResolveImageInfo) const override;

    template <typename RegionType>
    bool ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType* pRegions, VkFilter filter,
                              CMD_TYPE cmd_type) const;

    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit* pRegions, VkFilter filter) const override;

    bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) const override;

    bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) const override;

    template <typename RegionType>
    void RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType* pRegions, VkFilter filter);

    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions,
                                   VkFilter filter) override;

    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) override;

    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) override;

    bool ValidateCmdBufImageLayouts(const Location& loc, const CMD_BUFFER_STATE& cb_state,
                                    GlobalImageLayoutMap& overlayLayoutMap) const;

    void UpdateCmdBufImageLayouts(const CMD_BUFFER_STATE* cb_state);

    bool VerifyBoundMemoryIsValid(const DEVICE_MEMORY_STATE* mem_state, const LogObjectList& objlist,
                                  const VulkanTypedHandle& typed_handle, const char* api_name, const char* error_code) const;
    template <typename LocType>
    bool VerifyBoundMemoryIsValid(const DEVICE_MEMORY_STATE* mem_state, const LogObjectList& objlist,
                                  const VulkanTypedHandle& typed_handle, const LocType& location) const;

    bool ValidateLayoutVsAttachmentDescription(const debug_report_data* report_data, RenderPassCreateVersion rp_version,
                                               const VkImageLayout first_layout, const uint32_t attachment,
                                               const VkAttachmentDescription2& attachment_description) const;

    bool ValidateImageUsageFlags(VkCommandBuffer cb, IMAGE_STATE const& image_state, VkImageUsageFlags desired, bool strict,
                                 const char* msgCode, char const* func_name) const;

    bool ValidateImageFormatFeatureFlags(VkCommandBuffer cb, IMAGE_STATE const& image_state, VkFormatFeatureFlags2KHR desired,
                                         char const* func_name, const char* vuid) const;

    bool ValidateImageSubresourceLayers(const CMD_BUFFER_STATE& cb_state, const VkImageSubresourceLayers* subresource_layers,
                                        char const* func_name, char const* member, uint32_t i) const;

    template <typename HandleT>
    bool ValidateBufferUsageFlags(HandleT handle, const BUFFER_STATE& buffer_state, VkFlags desired, bool strict,
                                  const char* msgCode, char const* func_name, char const* usage_string) const;

    bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const override;

    bool PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkBufferView* pView) const override;

    bool ValidateImageAspectMask(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, bool is_image_disjoint,
                                 const char* func_name, const char* vuid = kVUID_Core_DrawState_InvalidImageAspect) const;

    bool ValidateCreateImageViewSubresourceRange(const IMAGE_STATE* image_state, bool is_imageview_2d_type,
                                                 const VkImageSubresourceRange& subresourceRange) const;

    bool ValidateCmdClearColorSubresourceRange(const IMAGE_STATE* image_state, const VkImageSubresourceRange& subresourceRange,
                                               const char* param_name) const;

    bool ValidateCmdClearDepthSubresourceRange(const IMAGE_STATE* image_state, const VkImageSubresourceRange& subresourceRange,
                                               const char* param_name) const;

    bool ValidateImageBarrierSubresourceRange(const Location& loc, const IMAGE_STATE* image_state,
                                              const VkImageSubresourceRange& subresourceRange) const;

    bool ValidateImageViewFormatFeatures(const IMAGE_STATE* image_state, const VkFormat view_format,
                                         const VkImageUsageFlags image_usage) const;

    bool PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkImageView* pView) const override;
    template <typename RegionType>
    bool ValidateCmdCopyBufferBounds(VkCommandBuffer cb, const BUFFER_STATE& src_buffer_state, const BUFFER_STATE& dst_buffer_state,
                                     uint32_t regionCount, const RegionType* pRegions, CMD_TYPE cmd_type) const;

    template <typename RegionType>
    bool ValidateImageBounds(VkCommandBuffer cb, const IMAGE_STATE& image_state, const uint32_t regionCount,
                             const RegionType* pRegions, const char* func_name, const char* msg_code) const;

    template <typename RegionType>
    bool ValidateBufferBounds(VkCommandBuffer cb, const IMAGE_STATE& image_state, const BUFFER_STATE& buff_state,
                              uint32_t regionCount, const RegionType* pRegions, const char* func_name, const char* msg_code) const;

    template <typename RegionType>
    bool ValidateCopyBufferImageTransferGranularityRequirements(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& img,
                                                                const RegionType* region, const uint32_t i, const char* function,
                                                                const char* vuid) const;

    bool ValidateImageMipLevel(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& img, uint32_t mip_level, const uint32_t i,
                               const char* function, const char* member, const char* vuid) const;

    bool ValidateImageArrayLayerRange(const CMD_BUFFER_STATE& cb_state, const IMAGE_STATE& img, const uint32_t base_layer,
                                      const uint32_t layer_count, const uint32_t i, const char* function, const char* member,
                                      const char* vuid) const;
    bool ValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, const char* apiName) const;

    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) override;

    void RecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo);
    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo) override;
    void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) override;

    template <typename RegionType>
    bool ValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                               const RegionType* pRegions, CMD_TYPE cmd_type) const;

    bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy* pRegions) const override;

    bool PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                          const VkCopyBufferInfo2KHR* pCopyBufferInfos) const override;

    bool PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfos) const override;

    template <typename RegionType>
    void RecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                             const RegionType* pRegions, CMD_TYPE cmd_type);
    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy* pRegions) override;
    void PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfos) override;
    void PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfos) override;

    bool PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView,
                                         const VkAllocationCallbacks* pAllocator) const override;

    bool PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) const override;

    bool PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                          const VkAllocationCallbacks* pAllocator) const override;

    bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                      uint32_t data) const override;

    template <typename RegionType>
    bool ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkBuffer dstBuffer, uint32_t regionCount, const RegionType* pRegions,
                                      CMD_TYPE cmd_type) const;

    bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkBuffer dstBuffer, uint32_t regionCount,
                                             const VkBufferImageCopy* pRegions) const override;

    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) override;

    bool PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo) const override;

    bool PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                              const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const override;

    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo) override;

    void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                            const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) override;

    template <typename RegionType>
    bool ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                      VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType* pRegions,
                                      CMD_TYPE cmd_type) const;

    bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                             VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkBufferImageCopy* pRegions) const override;

    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkBufferImageCopy* pRegions) override;

    bool PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                 const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo) const override;

    bool PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                              const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const override;

    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo) override;

    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                            const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) override;

    bool ValidateCreateImageANDROID(const debug_report_data* report_data, const VkImageCreateInfo* create_info) const;
    bool ValidateCreateImageViewANDROID(const VkImageViewCreateInfo* create_info) const;
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
                                            const VkAllocationCallbacks* pAllocator,
                                            VkPipelineCache* pPipelineCache) const override;
    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                void* cgpl_state) const override;
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               void* pipe_state) const override;
    bool PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                           uint32_t* pExecutableCount,
                                                           VkPipelineExecutablePropertiesKHR* pProperties) const override;
    bool ValidatePipelineExecutableInfo(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                        const char* caller_name, const char* feature_vuid) const;
    bool PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                           uint32_t* pStatisticCount,
                                                           VkPipelineExecutableStatisticKHR* pStatistics) const override;
    bool PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
        VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
        VkPipelineExecutableInternalRepresentationKHR* pStatistics) const override;
    bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator,
                                             VkPipelineLayout* pPipelineLayout) const override;
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, void* ads_state) const override;
    bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    void* pipe_state) const override;
    bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                     VkPipelineCache pipelineCache, uint32_t count,
                                                     const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     void* pipe_state) const override;
    bool PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                       VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                       VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                       VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                       VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                       VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                       uint32_t width, uint32_t height, uint32_t depth) const override;
    bool ValidateRaytracingShaderBindingTable(VkCommandBuffer commandBuffer, const char* rt_func_name,
                                              const char* vuid_single_device_memory, const char* vuid_binding_table_flag,
                                              const VkStridedDeviceAddressRegionKHR& binding_table,
                                              const char* binding_table_name) const;
    bool ValidateCmdTraceRaysKHR(bool isIndirect, VkCommandBuffer commandBuffer,
                                 const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                 const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                 const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                 const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable) const;
    bool PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                        const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                        const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                        const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                        const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                        uint32_t height, uint32_t depth) const override;
    bool PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                VkDeviceAddress indirectDeviceAddress) const override;
    bool PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer,
                                                 VkDeviceAddress indirectDeviceAddress) const override;
    bool PreCallValidateCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const override;
    void CreateDevice(const VkDeviceCreateInfo* pCreateInfo) override;
    bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize dataSize, const void* pData) const override;
    bool PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                                       VkQueue* pQueue) const override;
    bool PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) const override;
    bool PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkSamplerYcbcrConversion* pYcbcrConversion) const override;
    bool PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkSamplerYcbcrConversion* pYcbcrConversion) const override;
    bool PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const override;
    bool PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                               const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const override;
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) override;
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
                                    VkFence fence) const override;
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   VkResult result) override;
    bool ValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                              bool is_2khr) const;
    bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits,
                                        VkFence fence) const override;
    bool PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits,
                                     VkFence fence) const override;
    void RecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence, VkResult result);
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                       VkResult result) override;
    void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                    VkResult result) override;
    bool IsZeroAllocationSizeAllowed(const VkMemoryAllocateInfo* pAllocateInfo) const;
    bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const override;
    bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkFence* pFence) const override;
    bool PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const override;
    bool PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) const override;
    bool PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) const override;
    bool PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                         const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                         const VkAllocationCallbacks* pAllocator) const override;
    bool ValidatePerformanceQueryResults(const char* cmd_name, const QUERY_POOL_STATE* query_pool_state, uint32_t firstQuery,
                                         uint32_t queryCount, VkQueryResultFlags flags) const;
    bool ValidateGetQueryPoolPerformanceResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, void* pData,
                                                VkDeviceSize stride, VkQueryResultFlags flags, const char* apiName) const;
    bool PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                            size_t dataSize, void* pData, VkDeviceSize stride,
                                            VkQueryResultFlags flags) const override;
    bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                             const VkBindBufferMemoryInfo* pBindInfos) const override;
    bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                          const VkBindBufferMemoryInfo* pBindInfos) const override;
    bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem,
                                         VkDeviceSize memoryOffset) const override;
    bool PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                   VkMemoryRequirements* pMemoryRequirements) const override;
    bool PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements) const override;
    bool PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements) const override;
    bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                VkImageFormatProperties2* pImageFormatProperties) const override;
    bool PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                   VkImageFormatProperties2* pImageFormatProperties) const override;
    bool GetPhysicalDeviceImageFormatProperties(IMAGE_STATE& image_state, const char* vuid_string) const;
    bool PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                        const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                              const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                           const VkCommandBuffer* pCommandBuffers) const override;
    bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const override;
    bool PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) const override;
    bool PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                           const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) const override;
    bool PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) const override;
    bool PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                           const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                          const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkDescriptorSetLayout* pSetLayout) const override;
    bool PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                            VkDescriptorPoolResetFlags flags) const override;
    bool PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                           const VkDescriptorSet* pDescriptorSets) const override;
    bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                             const VkCopyDescriptorSet* pDescriptorCopies) const override;
    bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                           const VkCommandBufferBeginInfo* pBeginInfo) const override;
    bool ValidateRenderingInfoAttachment(const std::shared_ptr<const IMAGE_VIEW_STATE>& image_view, const char* attachment,
                                         const VkRenderingInfo* pRenderingInfo, const char* func_name) const;
    bool ValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo, CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer,
                                             const VkRenderingInfoKHR* pRenderingInfo) const override;
    bool PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) const override;
    bool ValidateRenderingAttachmentInfo(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                         const VkRenderingAttachmentInfo* pAttachments, const char* func_name) const;
    bool PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer) const override;
    bool PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer) const override;
    bool PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer) const override;
    bool PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) const override;
    bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                        VkPipeline pipeline) const override;
    bool ForbidInheritedViewportScissor(VkCommandBuffer, const CMD_BUFFER_STATE* cb_state, const char* vuid,
                                        const CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                       const VkViewport* pViewports) const override;
    bool PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                      const VkRect2D* pScissors) const override;
    bool PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                 uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) const override;

    bool PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                 const VkViewportWScalingNV* pViewportWScalings) const override;

    bool PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                  VkImageLayout imageLayout) const override;
    bool PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                           uint32_t viewportCount,
                                                           const VkShadingRatePaletteNV* pShadingRatePalettes) const override;
    bool ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV& triangles, const char* func_name) const;
    bool ValidateGeometryAABBNV(const VkGeometryAABBNV& geometry, const char* func_name) const;
    bool ValidateGeometryNV(const VkGeometryNV& geometry, const char* func_name) const;
    bool PreCallValidateCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkAccelerationStructureNV* pAccelerationStructure) const override;
    bool PreCallValidateCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkAccelerationStructureKHR* pAccelerationStructure) const override;
    bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                          const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) const override;
    bool PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                         size_t dataSize, void* pData) const override;
    bool ValidateAccelerationBuffers(uint32_t info_index, const VkAccelerationStructureBuildGeometryInfoKHR& info,
                                     const char* func_name) const;
    bool PreCallValidateCmdBuildAccelerationStructuresKHR(
        VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const override;

    bool PreCallValidateBuildAccelerationStructuresKHR(
        VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
        const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const override;
    bool PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                        VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                        VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                        VkBuffer scratch, VkDeviceSize scratchOffset) const override;
    bool PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                       VkAccelerationStructureNV src,
                                                       VkCopyAccelerationStructureModeNV mode) const override;
    bool PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                       const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                        const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const override;
    bool PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                             uint16_t lineStipplePattern) const override;
    bool PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                        float depthBiasSlopeFactor) const override;
    bool PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) const override;
    bool PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) const override;
    bool PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                 uint32_t compareMask) const override;
    bool PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                               uint32_t writeMask) const override;
    bool PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                               uint32_t reference) const override;
    bool PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                              const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                              const uint32_t* pDynamicOffsets) const override;
    bool PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet* pDescriptorWrites) const override;
    bool PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkIndexType indexType) const override;
    bool PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                             const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const override;
    bool ValidateVTGShaderStages(const CMD_BUFFER_STATE& cb_state, CMD_TYPE cmd_type) const;
    bool ValidateMeshShaderStage(const CMD_BUFFER_STATE& cb_state, CMD_TYPE cmd_type, bool is_NV) const;
    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance) const override;
    bool PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                        uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) const override;
    bool ValidateCmdDrawIndexedBufferSize(const CMD_BUFFER_STATE& cb_state, uint32_t indexCount, uint32_t firstIndex,
                                          const char* caller, const char* first_index_vuid) const;
    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const override;
    bool PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                               const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                               uint32_t firstInstance, uint32_t stride,
                                               const int32_t* pVertexOffset) const override;
    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride) const override;
    bool ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride, CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) const override;
    bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) const override;
    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) const override;
    bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const override;
    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                        uint32_t stride) const override;
    bool ValidateBaseGroups(const CMD_BUFFER_STATE& cb_state, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                            const char* apiName) const;
    bool PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                        uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                        uint32_t groupCountZ) const override;
    bool PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                           uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                           uint32_t groupCountZ) const override;
    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const override;
    bool ValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                              CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                        const VkDependencyInfoKHR* pDependencyInfo) const override;
    bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                     const VkDependencyInfo* pDependencyInfo) const override;
    bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const override;
    bool ValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                          VkPipelineStageFlags2KHR stageMask) const override;
    bool PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                       VkPipelineStageFlags2 stageMask) const override;
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount,
                                      const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
    bool ValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                const VkDependencyInfo* pDependencyInfos, CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          const VkDependencyInfoKHR* pDependencyInfos) const override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                       const VkDependencyInfo* pDependencyInfos) const override;
    void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                    VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                    uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) override;
    void RecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                              const VkDependencyInfo* pDependencyInfos, CMD_TYPE cmd_type);
    void PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                        const VkDependencyInfoKHR* pDependencyInfos) override;
    void PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                     const VkDependencyInfo* pDependencyInfos) override;
    void PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                     VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                     uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                     uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                     uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) override;
    void PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                         const VkDependencyInfoKHR* pDependencyInfos) override;
    void PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      const VkDependencyInfo* pDependencyInfos) override;
    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount,
                                           const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
    bool ValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                     CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                               const VkDependencyInfoKHR* pDependencyInfo) const override;
    bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) const override;
    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount,
                                         const VkImageMemoryBarrier* pImageMemoryBarriers) override;
    void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo) override;
    void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) override;

    void EnqueueVerifyBeginQuery(VkCommandBuffer, const QueryObject& query_obj, const CMD_TYPE cmd_type);
    bool PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                      VkFlags flags) const override;
    void PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkFlags flags) override;
    void EnqueueVerifyEndQuery(CMD_BUFFER_STATE& cb_state, const QueryObject& query_obj);
    bool PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) const override;
    void PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) override;
    bool ValidateQueryPoolIndex(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, const char* func_name,
                                const char* first_vuid, const char* sum_vuid) const;
    bool PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                          uint32_t queryCount) const override;
    bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags) const override;
    void PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                              uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                              VkQueryResultFlags flags) override;
    bool PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                         uint32_t offset, uint32_t size, const void* pValues) const override;
    bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t query) const override;

    bool ValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkQueryPool queryPool,
                                    uint32_t query, CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkQueryPool queryPool,
                                              uint32_t query) const override;
    bool PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                           uint32_t query) const override;
    void PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                        uint32_t slot) override;
    void PreCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkQueryPool queryPool,
                                            uint32_t query) override;
    void PreCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                         uint32_t query) override;
    void PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                  uint32_t accelerationStructureCount,
                                                                  const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                  VkQueryType queryType, VkQueryPool queryPool,
                                                                  uint32_t firstQuery) override;
    bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const override;
    bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const override;
    bool PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory mem, VkDeviceSize* pCommittedMem) const override;
    bool MsRenderedToSingleSampledValidateFBAttachments(uint32_t count, const VkAttachmentReference2* attachments,
                                                        const VkFramebufferCreateInfo* fbci, const VkRenderPassCreateInfo2* rpci,
                                                        uint32_t subpass, VkSampleCountFlagBits sample_count) const;
    bool ValidateFragmentShadingRateAttachments(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo) const;
    bool ValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                   const char* function_name) const;
    bool PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const override;
    bool PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const override;
    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           VkSubpassContents contents) const override;
    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents) override;
    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const VkSubpassBeginInfo* pSubpassBeginInfo) const override;
    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                            const VkSubpassBeginInfo* pSubpassBeginInfo) const override;
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfo* pSubpassBeginInfo) override;
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfo* pSubpassBeginInfo) override;
    bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const override;
    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) override;
    bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                           const VkSubpassEndInfo* pSubpassEndInfo) const override;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                          const VkSubpassEndInfo* pSubpassEndInfo) override;
    bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                        const VkSubpassEndInfo* pSubpassEndInfo) const override;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                       const VkSubpassEndInfo* pSubpassEndInfo) override;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const override;
    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) override;
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) const override;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) override;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) const override;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) override;
    class ViewportScissorInheritanceTracker;
    bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                           const VkCommandBuffer* pCommandBuffers) const override;
    bool PreCallValidateMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                                  void** ppData) const override;
    bool PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory mem) const override;
    bool PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                const VkMappedMemoryRange* pMemRanges) const override;
    bool PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                     const VkMappedMemoryRange* pMemRanges) const override;
    bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem,
                                        VkDeviceSize memoryOffset) const override;
    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                       VkResult result) override;
    bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                         const VkBindImageMemoryInfo* pBindInfos) const override;
    void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                        VkResult result) override;
    bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                            const VkBindImageMemoryInfo* pBindInfos) const override;
    void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                           VkResult result) override;
    bool PreCallValidateSetEvent(VkDevice device, VkEvent event) const override;
    bool PreCallValidateResetEvent(VkDevice device, VkEvent event) const override;
    bool PreCallValidateGetEventStatus(VkDevice device, VkEvent event) const override;
    bool ValidateSparseMemoryBind(const VkSparseMemoryBind& bind, VkDeviceSize resource_size, const char* func_name,
                                  const char* parameter_name) const;
    bool ValidateImageSubresourceSparseImageMemoryBind(IMAGE_STATE const& image_state, VkImageSubresource const& subresource,
                                                       uint32_t image_idx, uint32_t bind_idx) const;
    bool ValidateSparseImageMemoryBind(IMAGE_STATE const* image_state, VkSparseImageMemoryBind const& bind, uint32_t image_idx,
                                       uint32_t bind_idx) const;
    bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                        VkFence fence) const override;
    bool ValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo, const char* api_name) const;
    bool PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) const override;
    bool PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) const override;
    bool PreCallValidateImportSemaphoreFdKHR(VkDevice device,
                                             const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) const override;
    bool PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) const override;
#ifdef VK_USE_PLATFORM_FUCHSIA
    bool PreCallValidateImportSemaphoreZirconHandleFUCHSIA(
        VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) const override;

    void PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
        VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
        VkResult result) override;
    void PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                       const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                       zx_handle_t* pZirconHandle, VkResult result) override;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateImportSemaphoreWin32HandleKHR(
        VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) const override;
    bool PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                   HANDLE* pHandle) const override;
    bool PreCallValidateImportFenceWin32HandleKHR(
        VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) const override;
    bool PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                               HANDLE* pHandle) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) const override;
    bool PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) const override;

    bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const override;
    void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                          const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                             VkImage* pSwapchainImages, VkResult result) override;
    bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) const override;
    bool PreCallValidateReleaseSwapchainImagesEXT(VkDevice device,
                                                  const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) const override;
    bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                  const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkSwapchainKHR* pSwapchains) const override;
    bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                            VkFence fence, uint32_t* pImageIndex) const override;
    bool PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                             uint32_t* pImageIndex) const override;
    bool PreCallValidateWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId,
                                          uint64_t timeout) const override;
    bool PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                          const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                           VkSurfaceKHR surface, VkBool32* pSupported) const override;
    bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const override;
    bool PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const override;
    bool PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                        const void* pData) const override;
    bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                           VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                           const void* pData) const override;

    bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                            VkPipelineLayout layout, uint32_t set,
                                                            const void* pData) const override;
    bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                            uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const override;
    bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                       VkDisplayPlaneCapabilitiesKHR* pCapabilities) const override;
    bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                        const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                        VkDisplayPlaneCapabilities2KHR* pCapabilities) const override;
    bool PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkSurfaceKHR* pSurface) const override;
    bool PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const override;

    bool PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                VkQueryControlFlags flags, uint32_t index) const override;
    void PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                              VkQueryControlFlags flags, uint32_t index) override;
    bool PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                              uint32_t index) const override;
    void PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                            uint32_t index) override;

    bool PreCallValidateGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                              const VkVideoProfileInfoKHR* pVideoProfile,
                                                              VkVideoCapabilitiesKHR* pCapabilities) const override;
    bool PreCallValidateGetPhysicalDeviceVideoFormatPropertiesKHR(
        VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
        uint32_t* pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR* pVideoFormatProperties) const override;
    bool PreCallValidateCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator,
                                              VkVideoSessionKHR* pVideoSession) const override;
    bool PreCallValidateDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                               const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                  uint32_t bindSessionMemoryInfoCount,
                                                  const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) const override;
    bool PreCallValidateCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkVideoSessionParametersKHR* pVideoSessionParameters) const override;
    bool PreCallValidateUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                        const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) const override;
    bool PreCallValidateDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                         const VkAllocationCallbacks* pAllocator) const override;
    bool PreCallValidateCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer,
                                               const VkVideoBeginCodingInfoKHR* pBeginInfo) const override;
    bool PreCallValidateCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer,
                                             const VkVideoEndCodingInfoKHR* pEndCodingInfo) const override;
    bool PreCallValidateCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                 const VkVideoCodingControlInfoKHR* pCodingControlInfo) const override;
    bool PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo) const override;

    bool PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                  uint32_t discardRectangleCount,
                                                  const VkRect2D* pDiscardRectangles) const override;
    bool PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                 const VkSampleLocationsInfoEXT* pSampleLocationsInfo) const override;
    bool ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                      VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride,
                                      CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride) const override;
    bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride) const override;
    bool PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                    VkBuffer counterBuffer, VkDeviceSize counterBufferOffset,
                                                    uint32_t counterOffset, uint32_t vertexStride) const override;
    bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const override;
    bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride) const override;
    bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) const override;
    bool PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                            uint32_t groupCountZ) const override;
    bool PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    uint32_t drawCount, uint32_t stride) const override;
    bool PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride) const override;
    bool ValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, const char* apiName) const;
    bool PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const override;
    bool PreCallValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const override;
    bool PreCallValidateGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const override;
    bool ValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, const char* apiName) const;
    bool ValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                     const char* apiName) const;
    bool PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const override;
    bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                               const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const override;
    bool PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const override;
    bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                            const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const override;
    bool ValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask, CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) const override;
    bool PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) const override;
    bool ValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore sempahore, uint64_t* pValue, const char* apiName) const;
    bool PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore sempahore, uint64_t* pValue) const override;
    bool PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore sempahore, uint64_t* pValue) const override;
    bool ValidateComputeWorkGroupSizes(const SHADER_MODULE_STATE& module_state, const Instruction& entrypoint,
                                       const PipelineStageState& stage_state, uint32_t local_size_x, uint32_t local_size_y,
                                       uint32_t local_size_z) const;
    bool ValidateTaskMeshWorkGroupSizes(const SHADER_MODULE_STATE& module_state, const Instruction& entrypoint,
                                        const PipelineStageState& stage_state, uint32_t local_size_x, uint32_t local_size_y,
                                        uint32_t local_size_z) const;

    bool ValidateQueryRange(VkDevice device, VkQueryPool queryPool, uint32_t totalCount, uint32_t firstQuery, uint32_t queryCount,
                            const char* vuid_badfirst, const char* vuid_badrange, const char* apiName) const;
    bool ValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                const char* apiName) const;
    bool PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                          uint32_t queryCount) const override;
    bool PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                       uint32_t queryCount) const override;
    bool ValidateQueryPoolStride(const std::string& vuid_not_64, const std::string& vuid_64, const VkDeviceSize stride,
                                 const char* parameter_name, const uint64_t parameter_value, const VkQueryResultFlags flags) const;
    bool ValidateCmdDrawStrideWithStruct(VkCommandBuffer commandBuffer, const std::string& vuid, const uint32_t stride,
                                         const char* struct_name, const uint32_t struct_size) const;
    bool ValidateCmdDrawStrideWithBuffer(VkCommandBuffer commandBuffer, const std::string& vuid, const uint32_t stride,
                                         const char* struct_name, const uint32_t struct_size, const uint32_t drawCount,
                                         const VkDeviceSize offset, const BUFFER_STATE* buffer_state) const;
    bool PreCallValidateReleaseProfilingLockKHR(VkDevice device) const override;
    bool PreCallValidateCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkPrivateDataSlotEXT* pPrivateDataSlot) const override;
    bool PreCallValidateCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator,
                                              VkPrivateDataSlot* pPrivateDataSlot) const override;
    bool PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) const override;
    bool PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                           uint32_t bindingCount, const VkBuffer* pBuffers,
                                                           const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) const override;
    bool PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                     uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                     const VkDeviceSize* pCounterBufferOffsets) const override;
    bool PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                   uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                   const VkDeviceSize* pCounterBufferOffsets) const override;
    bool PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer,
                                                             uint32_t pipelineStackSize) const override;
    bool PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                             VkShaderGroupShaderKHR groupShader) const override;

    bool PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                 const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                 VkQueryType queryType, size_t dataSize, void* pData,
                                                                 size_t stride) const override;

    bool PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                    uint32_t accelerationStructureCount,
                                                                    const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                    VkQueryType queryType, VkQueryPool queryPool,
                                                                    uint32_t firstQuery) const override;

    bool PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                   uint32_t accelerationStructureCount,
                                                                   const VkAccelerationStructureNV* pAccelerationStructures,
                                                                   VkQueryType queryType, VkQueryPool queryPool,
                                                                   uint32_t firstQuery) const override;

    // Calculates the total number of shader groups taking libraries into account.
    uint32_t CalcTotalShaderGroupCount(const PIPELINE_STATE& pipeline) const;

    bool PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                           uint32_t groupCount, size_t dataSize, void* pData) const override;

    bool PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                        uint32_t groupCount, size_t dataSize,
                                                                        void* pData) const override;

    bool PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                  const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                  const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                  const uint32_t* pIndirectStrides,
                                                                  const uint32_t* const* ppMaxPrimitiveCounts) const override;
    bool ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR* pInfo, const char* api_name) const;
    bool PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                        const VkCopyAccelerationStructureInfoKHR* pInfo) const override;
    bool PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                     const VkCopyAccelerationStructureInfoKHR* pInfo) const override;
    bool PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(
        VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const override;
    bool PreCallValidateCopyMemoryToAccelerationStructureKHR(
        VkDevice device, VkDeferredOperationKHR deferredOperation,
        const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const override;
    bool PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(
        VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const override;
    bool ValidateExtendedDynamicState(const CMD_BUFFER_STATE& cb_state, const CMD_TYPE cmd_type, VkBool32 feature, const char* vuid,
                                      const char* feature_name) const;
    bool PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) const override;
    bool PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) const override;
    bool PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer,
                                                         VkBool32 rasterizerDiscardEnable) const override;
    bool PreCallValidateCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
                                                      VkBool32 rasterizerDiscardEnable) const override;
    bool PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) const override;
    bool PreCallValidateCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) const override;
    bool PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer,
                                                        VkBool32 primitiveRestartEnable) const override;
    bool PreCallValidateCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) const override;

    bool PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) const override;
    bool PreCallValidateCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) const override;
    bool PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) const override;
    bool PreCallValidateCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) const override;
    bool PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer,
                                                   VkPrimitiveTopology primitiveTopology) const override;
    bool PreCallValidateCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
                                                VkPrimitiveTopology primitiveTopology) const override;
    bool PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                   const VkViewport* pViewports) const override;
    bool PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                const VkViewport* pViewports) const override;
    bool PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                  const VkRect2D* pScissors) const override;
    bool PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                               const VkRect2D* pScissors) const override;
    bool ValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                       const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                       const VkDeviceSize* pStrides, CMD_TYPE cmd_type) const;
    bool PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                 const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                 const VkDeviceSize* pStrides) const override;
    bool PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                              const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                              const VkDeviceSize* pStrides) const override;
    bool PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) const override;
    bool PreCallValidateCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) const override;
    bool PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) const override;
    bool PreCallValidateCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) const override;
    bool PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) const override;
    bool PreCallValidateCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) const override;
    bool PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer,
                                                       VkBool32 depthBoundsTestEnable) const override;
    bool PreCallValidateCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) const override;
    bool PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) const override;
    bool PreCallValidateCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) const override;
    bool PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                           VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const override;
    bool PreCallValidateCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                        VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const override;
    bool PreCallValidateCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                          VkTessellationDomainOrigin domainOrigin) const override;
    bool PreCallValidateCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) const override;
    bool PreCallValidateCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) const override;
    bool PreCallValidateCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                      VkSampleCountFlagBits rasterizationSamples) const override;
    bool PreCallValidateCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                            const VkSampleMask* pSampleMask) const override;
    bool PreCallValidateCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer,
                                                       VkBool32 alphaToCoverageEnable) const override;
    bool PreCallValidateCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) const override;
    bool PreCallValidateCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) const override;
    bool PreCallValidateCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                  const VkBool32* pColorBlendEnables) const override;
    bool PreCallValidateCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                    uint32_t attachmentCount,
                                                    const VkColorBlendEquationEXT* pColorBlendEquations) const override;
    bool PreCallValidateCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                const VkColorComponentFlags* pColorWriteMasks) const override;
    bool PreCallValidateCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) const override;
    bool PreCallValidateCmdSetConservativeRasterizationModeEXT(
        VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode) const override;
    bool PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                  float extraPrimitiveOverestimationSize) const override;
    bool PreCallValidateCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) const override;
    bool PreCallValidateCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer,
                                                       VkBool32 sampleLocationsEnable) const override;
    bool PreCallValidateCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                    uint32_t attachmentCount,
                                                    const VkColorBlendAdvancedEXT* pColorBlendAdvanced) const override;
    bool PreCallValidateCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                     VkProvokingVertexModeEXT provokingVertexMode) const override;
    bool PreCallValidateCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                       VkLineRasterizationModeEXT lineRasterizationMode) const override;
    bool PreCallValidateCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) const override;
    bool PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) const override;
    bool PreCallValidateCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer,
                                                       VkBool32 viewportWScalingEnable) const override;
    bool PreCallValidateCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                const VkViewportSwizzleNV* pViewportSwizzles) const override;
    bool PreCallValidateCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) const override;
    bool PreCallValidateCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer,
                                                        uint32_t coverageToColorLocation) const override;
    bool PreCallValidateCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                       VkCoverageModulationModeNV coverageModulationMode) const override;
    bool PreCallValidateCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                              VkBool32 coverageModulationTableEnable) const override;
    bool PreCallValidateCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                        const float* pCoverageModulationTable) const override;
    bool PreCallValidateCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer,
                                                       VkBool32 shadingRateImageEnable) const override;
    bool PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                 VkBool32 representativeFragmentTestEnable) const override;
    bool PreCallValidateCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                      VkCoverageReductionModeNV coverageReductionMode) const override;
    bool PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkEvent* pEvent) const override;
    bool PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                     const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const override;
    bool PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                  const VkBool32* pColorWriteEnables) const override;
    bool PreCallValidateCmdSetVertexInputEXT(
        VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
        const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount,
        const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) const override;
    bool PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                  uint32_t customSampleOrderCount,
                                                  const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) const override;
    bool PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                        const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const override;
    bool PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                     const VkPerformanceMarkerInfoINTEL* pMarkerInfo) const override;
    bool PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                           const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) const override;
    bool PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                       const VkPerformanceOverrideInfoINTEL* pOverrideInfo) const override;
    bool PreCallValidateCmdBeginConditionalRenderingEXT(
        VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) const override;
    bool PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) const override;
    bool PreCallValidateReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) const override;
#endif

    bool ValidatePhysicalDeviceSurfaceSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const char* vuid,
                                              const char* func_name) const;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                              VkDeviceGroupPresentModeFlagsKHR* pModes) const override;
    bool PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                 uint32_t* pPresentModeCount,
                                                                 VkPresentModeKHR* pPresentModes) const override;
#endif
    bool PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                             VkDeviceGroupPresentModeFlagsKHR* pModes) const override;
    bool PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                              uint32_t* pRectCount, VkRect2D* pRects) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                 VkSurfaceCapabilities2EXT* pSurfaceCapabilities) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                 VkSurfaceCapabilities2KHR* pSurfaceCapabilities) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                            const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                            uint32_t* pSurfaceFormatCount,
                                                            VkSurfaceFormat2KHR* pSurfaceFormats) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           uint32_t* pSurfaceFormatCount,
                                                           VkSurfaceFormatKHR* pSurfaceFormats) const override;
    bool PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                uint32_t* pPresentModeCount,
                                                                VkPresentModeKHR* pPresentModes) const override;
    void PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                           size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                           VkResult result) override;
    bool ValidateGetImageSubresourceLayout(VkDevice device, const IMAGE_STATE& image_state, const VkImageSubresource& subresource,
                                           VkSubresourceLayout& layout, bool is_ext) const;
    bool PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                  VkSubresourceLayout* pLayout) const override;
    bool PreCallValidateGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2EXT* pSubresource,
                                                      VkSubresourceLayout2EXT* pLayout) const override;

    bool PreCallValidateGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                      VkDeviceSize* pLayoutSizeInBytes) const override;
    bool PreCallValidateGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                               VkDeviceSize* pOffset) const override;
    bool PreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                void* pData) const override;
    bool PreCallValidateGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                               void* pData) const override;
    bool PreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                   const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                   void* pData) const override;
    bool PreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                 const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                                 void* pData) const override;
    bool PreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
        VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData) const override;
    bool PreCallValidateCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                         VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                         const uint32_t* pBufferIndices,
                                                         const VkDeviceSize* pOffsets) const override;
    bool PreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                   VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                   uint32_t set) const override;
    bool PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                    const VkDescriptorBufferBindingInfoEXT* pBindingInfos) const override;
    bool ValidateDescriptorAddressInfoEXT(VkDevice device, const VkDescriptorAddressInfoEXT* address_info) const;
    bool PreCallValidateGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                         void* pDescriptor) const override;
#ifdef VK_USE_PLATFORM_METAL_EXT
    bool PreCallValidateExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo) const override;
#endif  // VK_USE_PLATFORM_METAL_EXT

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    bool PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(
        VkDevice device, const struct AHardwareBuffer* buffer,
        VkAndroidHardwareBufferPropertiesANDROID* pProperties) const override;
    bool PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                              const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                              struct AHardwareBuffer** pBuffer) const override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                       struct wl_display* display) const override;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                     uint32_t queueFamilyIndex) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   xcb_connection_t* connection,
                                                                   xcb_visualid_t visual_id) const override;
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                    Display* dpy, VisualID visualID) const override;
#endif  // VK_USE_PLATFORM_XLIB_KHR
    std::shared_ptr<CMD_BUFFER_STATE> CreateCmdBufferState(VkCommandBuffer cb, const VkCommandBufferAllocateInfo* create_info,
                                                           const COMMAND_POOL_STATE* pool) override {
        return std::static_pointer_cast<CMD_BUFFER_STATE>(std::make_shared<CORE_CMD_BUFFER_STATE>(this, cb, create_info, pool));
    }
};  // Class CoreChecks
