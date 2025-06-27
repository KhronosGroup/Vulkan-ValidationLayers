/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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

#include "best_practices/bp_constants.h"
// We pull in most the core state tracking files
// bp_state.h should NOT be included by any other header file
#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/push_constant_data.h"

class BestPractices;

namespace bp_state {

class ImageSubState : public vvl::ImageSubState {
  public:
    explicit ImageSubState(vvl::Image& img);

    struct Usage {
        IMAGE_SUBRESOURCE_USAGE_BP type;
        uint32_t queue_family_index;
    };

    Usage UpdateUsage(uint32_t array_layer, uint32_t mip_level, IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t queue_family);
    Usage GetUsage(uint32_t array_layer, uint32_t mip_level) const;
    IMAGE_SUBRESOURCE_USAGE_BP GetUsageType(uint32_t array_layer, uint32_t mip_level) const;
    uint32_t GetLastQueueFamily(uint32_t array_layer, uint32_t mip_level) const;

    std::array<bool, vvl::Image::kMaxPlanes> memory_requirements_checked = {};

    const bool sparse_metadata_required;  // Track if sparse metadata aspect is required for this image
    bool get_sparse_reqs_called{false};   // Track if GetImageSparseMemoryRequirements() has been called for this image
    bool sparse_metadata_bound{false};    // Track if sparse metadata aspect is bound to this image

  private:
    void SetupUsages();
    // A 2d vector for all the array layers and mip levels.
    // This does not split usages per aspect.
    // Aspects are generally read and written together,
    // and tracking them independently could be misleading.
    // second/uint32_t is last queue family usage
    std::vector<std::vector<Usage>> usages_;
};

static inline ImageSubState& SubState(vvl::Image& img) {
    return *static_cast<ImageSubState*>(img.SubState(LayerObjectTypeBestPractices));
}

static inline const ImageSubState& SubState(const vvl::Image& img) {
    return *static_cast<const ImageSubState*>(img.SubState(LayerObjectTypeBestPractices));
}

struct AttachmentInfo {
    uint32_t framebufferAttachment;
    VkImageAspectFlags aspects;

    AttachmentInfo(uint32_t framebufferAttachment_, VkImageAspectFlags aspects_)
        : framebufferAttachment(framebufferAttachment_), aspects(aspects_) {}
};

// used to track state regarding render pass heuristic checks
// TODO - make vvl::RenderPassSubState instead
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

    bool has_draw_cmd = false;
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

    vvl::unordered_map<VkImage, ZcullTree> zcull_per_image;
    ZcullScope zcull_scope;
    ZcullDirection zcull_direction = ZcullDirection::Unknown;

    VkCompareOp depth_compare_op = VK_COMPARE_OP_NEVER;
    bool depth_test_enable = false;
};

class CommandBufferSubState : public vvl::CommandBufferSubState {
  public:
    explicit CommandBufferSubState(vvl::CommandBuffer& cb, BestPractices& validator);
    BestPractices& validator;

    RenderPassState render_pass_state;
    CommandBufferStateNV nv;
    uint64_t num_submits = 0;
    uint32_t small_indexed_draw_call_count = 0;

    std::vector<PushConstantData> push_constant_data_chunks;

    // This function used to not be empty. It has been left empty because
    // the logic to decide to call this function is not simple, so adding this
    // function back could tedious.
    void UnbindResources() {}

    void Destroy() final;
    void Reset(const Location& loc) final;

    void RecordExecuteCommand(vvl::CommandBuffer& secondary_command_buffer) final;
    void RecordActionCommand(LastBound& last_bound, const Location& loc) final;

    void RecordPushConstants(VkPipelineLayout layout, VkShaderStageFlags stage_flags, uint32_t offset, uint32_t size,
                             const void* values) final;
    void ClearPushConstants() final;

    void RecordBeginRendering(const VkRenderingInfo& rendering_info) final;
    void RecordBeginRenderPass(const VkRenderPassBeginInfo& render_pass_begin) final;
    void RecordNextSubpass() final;
    void RecordEndRendering(const VkRenderingEndInfoEXT* pRenderingEndInfo) final;
    void RecordEndRenderPass() final;

    void RecordCopyImage(vvl::Image& src_image_state, vvl::Image& dst_image_state, VkImageLayout src_image_layout,
                         VkImageLayout dst_image_layout, uint32_t region_count, const VkImageCopy* regions,
                         const Location& loc) final;
    void RecordCopyImage2(vvl::Image& src_image_state, vvl::Image& dst_image_state, VkImageLayout src_image_layout,
                          VkImageLayout dst_image_layout, uint32_t region_count, const VkImageCopy2* regions,
                          const Location& loc) final;
    void RecordCopyBufferToImage(vvl::Image& dst_image_state, VkImageLayout dst_image_layout, uint32_t region_count,
                                 const VkBufferImageCopy* regions, const Location& loc) final;
    void RecordCopyBufferToImage2(vvl::Image& dst_image_state, VkImageLayout dst_image_layout, uint32_t region_count,
                                  const VkBufferImageCopy2* regions, const Location& loc) final;
    void RecordCopyImageToBuffer(vvl::Image& src_image_state, VkImageLayout src_image_layout, uint32_t region_count,
                                 const VkBufferImageCopy* regions, const Location& loc) final;
    void RecordCopyImageToBuffer2(vvl::Image& src_image_state, VkImageLayout src_image_layout, uint32_t region_count,
                                  const VkBufferImageCopy2* regions, const Location& loc) final;
    void RecordBlitImage(vvl::Image& src_image_state, vvl::Image& dst_image_state, VkImageLayout src_image_layout,
                         VkImageLayout dst_image_layout, uint32_t region_count, const VkImageBlit* regions,
                         const Location& loc) final;
    void RecordBlitImage2(vvl::Image& src_image_state, vvl::Image& dst_image_state, VkImageLayout src_image_layout,
                          VkImageLayout dst_image_layout, uint32_t region_count, const VkImageBlit2* regions,
                          const Location& loc) final;
    void RecordResolveImage(vvl::Image& src_image_state, vvl::Image& dst_image_state, uint32_t region_count,
                            const VkImageResolve* regions, const Location& loc) final;
    void RecordResolveImage2(vvl::Image& src_image_state, vvl::Image& dst_image_state, uint32_t region_count,
                             const VkImageResolve2* regions, const Location& loc) final;
    void RecordClearColorImage(vvl::Image& image_state, VkImageLayout image_layout, const VkClearColorValue* color_values,
                               uint32_t range_count, const VkImageSubresourceRange* ranges, const Location& loc) final;
    void RecordClearDepthStencilImage(vvl::Image& image_state, VkImageLayout image_layout,
                                      const VkClearDepthStencilValue* depth_stencil_values, uint32_t range_count,
                                      const VkImageSubresourceRange* ranges, const Location& loc) final;
    void RecordClearAttachments(uint32_t attachment_count, const VkClearAttachment* pAttachments, uint32_t rect_count,
                                const VkClearRect* pRects, const Location& loc) final;

    void RecordSetEvent(vvl::Func command, VkEvent event, VkPipelineStageFlags2 stageMask,
                        const VkDependencyInfo* dependency_info) final;
    void RecordResetEvent(vvl::Func command, VkEvent event, VkPipelineStageFlags2 stageMask) final;

    void RecordBindPipeline(VkPipelineBindPoint bind_point, vvl::Pipeline& pipeline) final;

    void Submit(vvl::Queue& queue_state, uint32_t perf_submit_pass, const Location& loc) final;

    // Move to private when possible
    void RecordAttachmentAccess(uint32_t attachment, VkImageAspectFlags aspects);
    void RecordAttachmentClearAttachments(uint32_t fb_attachment, uint32_t color_attachment, VkImageAspectFlags aspects,
                                          uint32_t rectCount, const VkClearRect* pRects);
    void RecordSetDepthTestStateNV(VkCompareOp new_depth_compare_op, bool new_depth_test_enable);

    struct SignalingInfo {
        // True, if the event's first state change within a command buffer is a signal (SetEvent)
        // rather than an unsignal (ResetEvent). It is used to do validation on the boundary
        // between two command buffers.
        const bool first_state_change_is_signal = false;

        // Tracks how the event signaling state changes as the command buffer recording progresses.
        // When recording is finished, this is the event state "at the end of the command buffer".
        bool signaled = false;

        explicit SignalingInfo(bool signal) : first_state_change_is_signal(signal), signaled(signal) {}
    };
    vvl::unordered_map<VkEvent, SignalingInfo> event_signaling_state;

    using QueueCallback = std::function<bool(const class vvl::Queue& queue_state, const vvl::CommandBuffer& cb_state)>;
    std::vector<QueueCallback> queue_submit_functions;
    // Used by some layers to defer actions until vkCmdEndRenderPass time.
    // Layers using this are responsible for inserting the callbacks into queue_submit_functions.
    std::vector<QueueCallback> queue_submit_functions_after_render_pass;

    void RecordBindZcullScopeNV(VkImage depth_attachment, const VkImageSubresourceRange& subresource_range);
    void RecordUnbindZcullScopeNV();
    void RecordResetScopeZcullDirectionNV();
    void RecordResetZcullDirectionNV(VkImage depth_image, const VkImageSubresourceRange& subresource_range);
    void RecordSetZcullDirectionNV(VkImage depth_image, const VkImageSubresourceRange& subresource_range, ZcullDirection mode);
    void RecordSetScopeZcullDirectionNV(ZcullDirection mode);
    void RecordZcullDrawNV();
    void RecordCmdDrawTypeNVIDIA();

  private:
    void ResetCBState();
    void RecordBeginRenderingCommon(const VkRenderPassBeginInfo* pRenderPassBegin, const VkRenderingInfo* pRenderingInfo);
    void RecordEndRenderingCommon(const vvl::RenderPass& rp_state);
};

static inline CommandBufferSubState& SubState(vvl::CommandBuffer& cb) {
    return *static_cast<CommandBufferSubState*>(cb.SubState(LayerObjectTypeBestPractices));
}

static inline const CommandBufferSubState& SubState(const vvl::CommandBuffer& cb) {
    return *static_cast<const CommandBufferSubState*>(cb.SubState(LayerObjectTypeBestPractices));
}

}  // namespace bp_state
