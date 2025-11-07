/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
#include "cc_submit.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/event_map.h"
#include "state_tracker/shader_stage_state.h"

class CoreChecks;

namespace core {

// CommandBuffer is over 3 times larger than the next largest state object struct, but the majority of the state is only used in
// CoreChecks. This state object is used by everyone else (best practice, sync val, GPU-AV, etc). For this reason, we have
// CommandBuffer object only for core and keep only the most basic items in the parent class
class CommandBufferSubState : public vvl::CommandBufferSubState {
  public:
    CommandBufferSubState(vvl::CommandBuffer &cb, CoreChecks &validator);

    void Begin(const VkCommandBufferBeginInfo &begin_info) final;

    void RecordActionCommand(LastBound &last_bound, const Location &loc) final;

    void RecordBindPipeline(VkPipelineBindPoint bind_point, vvl::Pipeline &pipeline) final;
    void RecordSetViewport(uint32_t first_viewport, uint32_t viewport_count) final;
    void RecordSetViewportWithCount(uint32_t viewport_count) final;
    void RecordSetScissor(uint32_t first_scissor, uint32_t scissor_count) final;
    void RecordSetScissorWithCount(uint32_t scissor_count) final;

    void RecordBeginRenderPass(const VkRenderPassBeginInfo &render_pass_begin, const VkSubpassBeginInfo &subpass_begin_info,
                               const Location &loc) final;
    void RecordNextSubpass(const VkSubpassBeginInfo &subpass_begin_info, const VkSubpassEndInfo *subpass_end_info,
                           const Location &loc) final;
    void RecordEndRendering(const VkRenderingEndInfoEXT *pRenderingEndInfo) final;
    void RecordEndRenderPass(const VkSubpassEndInfo *subpass_end_info, const Location &loc) final;

    void RecordCopyBuffer(vvl::Buffer &src_buffer_state, vvl::Buffer &dst_buffer_state, uint32_t region_count,
                          const VkBufferCopy *regions, const Location &loc) final;
    void RecordCopyBuffer2(vvl::Buffer &src_buffer_state, vvl::Buffer &dst_buffer_state, uint32_t region_count,
                           const VkBufferCopy2 *regions, const Location &loc) final;
    void RecordCopyImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                         VkImageLayout dst_image_layout, uint32_t region_count, const VkImageCopy *regions,
                         const Location &loc) final;
    void RecordCopyImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                          VkImageLayout dst_image_layout, uint32_t region_count, const VkImageCopy2 *regions,
                          const Location &loc) final;
    void RecordCopyBufferToImage(vvl::Buffer &src_buffer_state, vvl::Image &dst_image_state, VkImageLayout dst_image_layout,
                                 uint32_t region_count, const VkBufferImageCopy *regions, const Location &loc) final;
    void RecordCopyBufferToImage2(vvl::Buffer &src_buffer_state, vvl::Image &dst_image_state, VkImageLayout dst_image_layout,
                                  uint32_t region_count, const VkBufferImageCopy2 *regions, const Location &loc) final;
    void RecordCopyImageToBuffer(vvl::Image &src_image_state, vvl::Buffer &dst_buffer_state, VkImageLayout src_image_layout,
                                 uint32_t region_count, const VkBufferImageCopy *regions, const Location &loc) final;
    void RecordCopyImageToBuffer2(vvl::Image &src_image_state, vvl::Buffer &dst_buffer_state, VkImageLayout src_image_layout,
                                  uint32_t region_count, const VkBufferImageCopy2 *regions, const Location &loc) final;
    void RecordBlitImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                         VkImageLayout dst_image_layout, uint32_t region_count, const VkImageBlit *regions,
                         const Location &loc) final;
    void RecordBlitImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                          VkImageLayout dst_image_layout, uint32_t region_count, const VkImageBlit2 *regions,
                          const Location &loc) final;
    void RecordClearColorImage(vvl::Image &image_state, VkImageLayout image_layout, const VkClearColorValue *color_values,
                               uint32_t range_count, const VkImageSubresourceRange *ranges, const Location &loc) final;
    void RecordClearDepthStencilImage(vvl::Image &image_state, VkImageLayout image_layout,
                                      const VkClearDepthStencilValue *depth_stencil_values, uint32_t range_count,
                                      const VkImageSubresourceRange *ranges, const Location &loc) final;
    void RecordClearAttachments(uint32_t attachment_count, const VkClearAttachment *pAttachments, uint32_t rect_count,
                                const VkClearRect *pRects, const Location &loc) final;

    void RecordSetEvent(VkEvent event, VkPipelineStageFlags2 stageMask, const VkDependencyInfo *dependency_info) final;
    void RecordResetEvent(VkEvent event, VkPipelineStageFlags2 stageMask) final;
    void RecordWaitEvents(uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags2 src_stage_mask,
                          const VkDependencyInfo *dependency_info, const Location &loc) final;
    void RecordBarriers(uint32_t buffer_barrier_count, const VkBufferMemoryBarrier *buffer_barriers, uint32_t image_barrier_count,
                        const VkImageMemoryBarrier *image_barriers, VkPipelineStageFlags src_stage_mask,
                        VkPipelineStageFlags dst_stage_mask, const Location &loc) final;
    void RecordBarriers2(const VkDependencyInfo &dep_info, const Location &loc) final;

    void RecordBeginQuery(const QueryObject &query_obj, const Location &loc) final;
    void RecordEndQuery(const QueryObject &query_obj, const Location &loc) final;
    void RecordEndQueries(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) final;
    void RecordWriteTimestamp(const QueryObject &query_obj, const Location &loc) final;
    void RecordResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, bool is_perf_query,
                              const Location &loc) final;
    void RecordCopyQueryPoolResults(vvl::QueryPool &pool_state, vvl::Buffer &dst_buffer_state, uint32_t first_query,
                                    uint32_t query_count, VkDeviceSize dst_offset, VkDeviceSize stride, VkQueryResultFlags flags,
                                    const Location &loc) final;
    void RecordWriteAccelerationStructuresProperties(VkQueryPool queryPool, uint32_t firstQuery,
                                                     uint32_t accelerationStructureCount, const Location &loc) final;
    void RecordVideoInlineQueries(const VkVideoInlineQueryInfoKHR &query_info) final;

    void RecordBeginVideoCoding(vvl::VideoSession &vs_state, const VkVideoBeginCodingInfoKHR &begin_info,
                                const Location &loc) final;
    void RecordControlVideoCoding(vvl::VideoSession &vs_state, const VkVideoCodingControlInfoKHR &control_info,
                                  const Location &loc) final;
    void RecordDecodeVideo(vvl::VideoSession &vs_state, const VkVideoDecodeInfoKHR &decode_info, const Location &loc) final;
    void RecordEncodeVideo(vvl::VideoSession &vs_state, const VkVideoEncodeInfoKHR &encode_info, const Location &loc) final;

    // Only need to retire for core checks to track queries
    void Retire(uint32_t perf_submit_pass, const std::function<bool(const QueryObject &)> &is_query_updated_after);

    void Reset(const Location &loc) final;
    void Destroy() final;

    void RecordExecuteCommand(vvl::CommandBuffer &secondary_command_buffer, uint32_t cmd_index, const Location &loc) final;

    // Called from the Queue state
    void SubmitTimeValidate();
    // Called from the Command Buffer state
    void Submit(vvl::Queue &queue_state, uint32_t perf_submit_pass, const Location &loc) final;

    CoreChecks &validator;

    uint32_t nesting_level;  // VK_EXT_nested_command_buffer

    struct Viewport {
        uint32_t mask;
        uint32_t count_mask;

        // Bits set when binding graphics pipeline defining corresponding static state, or executing any secondary command buffer.
        // Bits unset by calling a corresponding vkCmdSet[State] cmd.
        uint32_t trashed_mask;
        bool trashed_count;

        bool used_dynamic_count;  // true if any draw recorded used VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT

        // If VK_NV_inherited_viewport_scissor is enabled and VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D is
        // true, then is the nonempty list of viewports passed in pViewportDepths. Otherwise, this is empty.
        std::vector<VkViewport> inherited_depths;
    } viewport;

    struct Scissor {
        uint32_t mask;
        uint32_t count_mask;

        uint32_t trashed_mask;
        bool trashed_count;

        bool used_dynamic_count;  // true if any draw recorded used VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT
    } scissor;

    uint32_t used_viewport_scissor_count;

    QFOTransferBarrierSets<QFOBufferTransferBarrier> qfo_transfer_buffer_barriers;
    QFOTransferBarrierSets<QFOImageTransferBarrier> qfo_transfer_image_barriers;
    const QFOTransferBarrierSets<QFOImageTransferBarrier> &GetQFOBarrierSets(const QFOImageTransferBarrier &type_tag) const {
        return qfo_transfer_image_barriers;
    }
    const QFOTransferBarrierSets<QFOBufferTransferBarrier> &GetQFOBarrierSets(const QFOBufferTransferBarrier &type_tag) const {
        return qfo_transfer_buffer_barriers;
    }

    // used for VK_EXT_fragment_density_map_offset
    // currently need to hold in Command buffer because it can be a suspended renderpass
    std::vector<VkOffset2D> fragment_density_offsets;

    // Validation functions run at primary CB queue submit time
    using QueueCallback = std::function<bool(const class vvl::Queue &queue_state, const vvl::CommandBuffer &cb_state)>;
    std::vector<QueueCallback> queue_submit_functions;

    // The subresources from dynamic rendering barriers that can't be validated during record time.
    vvl::unordered_map<VkImage, std::vector<std::pair<VkImageSubresourceRange, vvl::LocationCapture>>>
        submit_validate_dynamic_rendering_barrier_subresources;

    using EventCallback = std::function<bool(vvl::CommandBuffer &cb_state, bool do_validate, EventMap &local_event_signal_info,
                                             VkQueue waiting_queue, const Location &loc)>;
    std::vector<EventCallback> event_updates;

    // Validation functions run when secondary CB is executed in primary
    std::vector<
        std::function<bool(const vvl::CommandBuffer &secondary, const vvl::CommandBuffer *primary, const vvl::Framebuffer *)>>
        cmd_execute_commands_functions;

    std::vector<std::function<bool(vvl::CommandBuffer &cb_state, bool do_validate, VkQueryPool &first_perf_query_pool,
                                   uint32_t perf_query_pass, QueryMap *local_query_to_state_map)>>
        query_updates;

  private:
    void ResetCBState();
    void UpdateActionPipelineState(LastBound &last_bound, const vvl::Pipeline &pipeline_state);
    void UpdateActionShaderObjectState(LastBound &last_bound);
    void UpdateActiveSlotsState(LastBound &last_bound, const ActiveSlotMap &active_slots);

    // Funnel because Image/Buffer copies have 2 variations for the regions
    template <typename RegionType>
    void RecordCopyBufferCommon(vvl::Buffer &src_buffer_state, vvl::Buffer &dst_buffer_state, uint32_t region_count,
                                const RegionType *regions, const Location &loc);

    void EnqueueVerifyVideoSessionInitialized(vvl::VideoSession &vs_state, const Location &loc, const char *vuid);
    void EnqueueVerifyVideoInlineQueryUnavailable(const VkVideoInlineQueryInfoKHR &query_info, vvl::Func command);
};

static inline CommandBufferSubState &SubState(vvl::CommandBuffer &cb) {
    return *static_cast<CommandBufferSubState *>(cb.SubState(LayerObjectTypeCoreValidation));
}
static inline const CommandBufferSubState &SubState(const vvl::CommandBuffer &cb) {
    return *static_cast<const CommandBufferSubState *>(cb.SubState(LayerObjectTypeCoreValidation));
}

class QueueSubState : public vvl::QueueSubState {
  public:
    QueueSubState(CoreChecks &core_checks, vvl::Queue& q);

    void PreSubmit(std::vector<vvl::QueueSubmission> &submissions) override;

    // Override Retire to validate submissions in the order defined by synchronization
    void Retire(vvl::QueueSubmission&) override;

  private:
    QueueSubmissionValidator queue_submission_validator_;
};

}  // namespace core
