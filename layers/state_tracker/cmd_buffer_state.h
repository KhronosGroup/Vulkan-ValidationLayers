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
#include "utils/hash_vk_types.h"
#include "state_tracker/state_object.h"
#include "state_tracker/image_layout_map.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/query_state.h"
#include "state_tracker/vertex_index_buffer_state.h"
#include "containers/qfo_transfer.h"
#include "containers/custom_containers.h"
#include "generated/dynamic_state_helper.h"

struct SubpassInfo;
class CoreChecks;
class ValidationStateTracker;

namespace vvl {
class Bindable;
class Buffer;
class Framebuffer;
class RenderPass;
class VideoSession;
class VideoSessionParameters;
}  // namespace vvl

#ifdef VK_USE_PLATFORM_METAL_EXT
static bool GetMetalExport(const VkEventCreateInfo *info) {
    bool retval = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif  // VK_USE_PLATFORM_METAL_EXT

// Only CoreChecks uses this, but the state tracker stores it.
constexpr static auto kInvalidLayout = image_layout_map::kInvalidLayout;
using ImageSubresourceLayoutMap = image_layout_map::ImageSubresourceLayoutMap;
typedef vvl::unordered_map<VkEvent, VkPipelineStageFlags2KHR> EventToStageMap;

enum class CbState {
    New,                // Newly created CB w/o any cmds
    Recording,          // BeginCB has been called on this CB
    Recorded,           // EndCB has been called on this CB
    InvalidComplete,    // had a complete recording, but was since invalidated
    InvalidIncomplete,  // fouled before recording was completed
};

namespace vvl {

class Event : public StateObject {
  public:
    int write_in_use;
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_event_export;
#endif  // VK_USE_PLATFORM_METAL_EXT
    const VkEventCreateFlags flags;

    // Source stage specified by the "set event" command
    VkPipelineStageFlags2 signal_src_stage_mask = VK_PIPELINE_STAGE_2_NONE;

    // Queue that signaled this event. It's null if event was signaled from the host
    VkQueue signaling_queue = VK_NULL_HANDLE;

    Event(VkEvent handle, const VkEventCreateInfo *pCreateInfo)
        : StateObject(handle, kVulkanObjectTypeEvent),
          write_in_use(0),
#ifdef VK_USE_PLATFORM_METAL_EXT
          metal_event_export(GetMetalExport(pCreateInfo)),
#endif  // VK_USE_PLATFORM_METAL_EXT
          flags(pCreateInfo->flags) {
    }

    VkEvent VkHandle() const { return handle_.Cast<VkEvent>(); }
};

// Track command pools and their command buffers
class CommandPool : public StateObject {
  public:
    ValidationStateTracker &dev_data;
    const VkCommandPoolCreateFlags createFlags;
    const uint32_t queueFamilyIndex;
    const VkQueueFlags queue_flags;
    const bool unprotected;  // can't be used for protected memory
    // Cmd buffers allocated from this pool
    vvl::unordered_map<VkCommandBuffer, CommandBuffer *> commandBuffers;

    CommandPool(ValidationStateTracker &dev, VkCommandPool handle, const VkCommandPoolCreateInfo *pCreateInfo, VkQueueFlags flags);
    virtual ~CommandPool() { Destroy(); }

    VkCommandPool VkHandle() const { return handle_.Cast<VkCommandPool>(); }

    void Allocate(const VkCommandBufferAllocateInfo *create_info, const VkCommandBuffer *command_buffers);
    void Free(uint32_t count, const VkCommandBuffer *command_buffers);
    void Reset();

    void Destroy() override;
};

class CommandBuffer : public RefcountedStateObject {
    using Func = vvl::Func;
  public:
    struct LayoutState {
        StateObject::IdType id;
        std::shared_ptr<ImageSubresourceLayoutMap> map;
    };
    using ImageLayoutMap = vvl::unordered_map<VkImage, LayoutState>;
    using AliasedLayoutMap = vvl::unordered_map<const GlobalImageLayoutRangeMap *, std::shared_ptr<ImageSubresourceLayoutMap>>;

    VkCommandBufferAllocateInfo allocate_info;
    VkCommandBufferBeginInfo beginInfo;
    VkCommandBufferInheritanceInfo inheritanceInfo;
    // since command buffers can only be destroyed by their command pool, this does not need to be a shared_ptr
    const vvl::CommandPool *command_pool;
    ValidationStateTracker &dev_data;
    bool unprotected;  // can't be used for protected memory
    bool hasRenderPassInstance;
    bool suspendsRenderPassInstance;
    bool resumesRenderPassInstance;

    // Track if certain commands have been called at least once in lifetime of the command buffer
    // primary command buffers values are set true if a secondary command buffer has a command
    bool has_draw_cmd;
    bool has_dispatch_cmd;
    bool has_trace_rays_cmd;
    bool has_build_as_cmd;

    CbState state;           // Track cmd buffer update state
    uint64_t command_count;  // Number of commands recorded. Currently only used with VK_KHR_performance_query
    uint64_t submitCount;    // Number of times CB has been submitted
    typedef uint64_t ImageLayoutUpdateCount;
    ImageLayoutUpdateCount image_layout_change_count;  // The sequence number for changes to image layout (for cached validation)

    // Track status of all vkCmdSet* calls, if 1, means it was set
    struct DynamicStateStatus {
        CBDynamicFlags cb;        // for lifetime of CommandBuffer
        CBDynamicFlags pipeline;  // for lifetime since last bound pipeline
    } dynamic_state_status;

    // These are values that are being set with vkCmdSet* tied to a command buffer
    struct DynamicStateValue {
        // VK_DYNAMIC_STATE_STENCIL_WRITE_MASK
        uint32_t write_mask_front;
        uint32_t write_mask_back;
        // VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE
        bool depth_write_enable;
        // VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE
        bool depth_test_enable;
        // VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE
        bool depth_bounds_test_enable;
        // VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE
        bool stencil_test_enable;
        // VK_DYNAMIC_STATE_STENCIL_OP
        VkStencilOp fail_op_front;
        VkStencilOp pass_op_front;
        VkStencilOp depth_fail_op_front;
        VkStencilOp fail_op_back;
        VkStencilOp pass_op_back;
        VkStencilOp depth_fail_op_back;
        // VK_DYNAMIC_STATE_CULL_MODE
        VkCullModeFlags cull_mode;
        // VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
        VkPrimitiveTopology primitive_topology;
        // VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT
        VkSampleLocationsInfoEXT sample_locations_info;
        // VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT
        bool discard_rectangle_enable;
        // VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT
        // maxDiscardRectangles is at max 8 on all known implementations currently
        std::bitset<32> discard_rectangles;
        // VK_DYNAMIC_STATE_POLYGON_MODE_EXT
        VkPolygonMode polygon_mode;
        // VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT
        VkSampleCountFlagBits rasterization_samples;
        // VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT
        uint32_t rasterization_stream;
        // VK_DYNAMIC_STATE_SAMPLE_MASK_EXT
        VkSampleCountFlagBits samples_mask_samples;
        // VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_KHR
        VkLineRasterizationModeKHR line_rasterization_mode;
        // VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT
        bool stippled_line_enable;
        // VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV
        bool coverage_to_color_enable;
        // VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV
        uint32_t coverage_to_color_location;
        // VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV
        VkCoverageModulationModeNV coverage_modulation_mode;
        // VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV
        bool coverage_modulation_table_enable;
        // VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV
        bool shading_rate_image_enable;
        // VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE
        bool rasterizer_discard_enable;
        // VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE
        bool depth_bias_enable = false;
        // VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT
        bool alpha_to_coverage_enable;
        // VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT
        bool logic_op_enable;
        // VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR
        VkExtent2D fragment_size;

        uint32_t color_write_enable_attachment_count;

        // maxColorAttachments is at max 8 on all known implementations currently
        std::bitset<32> color_blend_enable_attachments;              // VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT
        std::bitset<32> color_blend_enabled;                         // VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT
        std::bitset<32> color_blend_equation_attachments;            // VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT
        std::bitset<32> color_write_mask_attachments;                // VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT
        std::bitset<32> color_blend_advanced_attachments;            // VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT
        std::bitset<32> color_write_enabled;                         // VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT
        std::vector<VkColorBlendEquationEXT> color_blend_equations;  // VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT
        std::vector<VkColorComponentFlags> color_write_masks;        // VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT

        // VK_DYNAMIC_STATE_VERTEX_INPUT_EXT
        std::vector<uint32_t> vertex_binding_descriptions_divisor;
        std::vector<VkVertexInputAttributeDescription2EXT> vertex_attribute_descriptions;

        // VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT
        VkConservativeRasterizationModeEXT conservative_rasterization_mode;
        // VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT
        bool sample_locations_enable;
        // VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT
        VkImageAspectFlags attachment_feedback_loop_enable;

        // VK_DYNAMIC_STATE_VIEWPORT
        std::vector<VkViewport> viewports;
        // and VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT
        uint32_t viewport_count;
        // VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT
        uint32_t scissor_count;
        // VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV
        std::vector<VkViewportWScalingNV> viewport_w_scalings;
        uint32_t viewport_w_scaling_first;
        uint32_t viewport_w_scaling_count;
        // VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE
        bool viewport_w_scaling_enable;
        // VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV
        uint32_t viewport_swizzle_count;
        // VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV
        uint32_t shading_rate_palette_count;
        // VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV
        uint32_t exclusive_scissor_enable_first;
        uint32_t exclusive_scissor_enable_count;
        std::vector<VkBool32> exclusive_scissor_enables;
        // VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV
        uint32_t exclusive_scissor_first;
        uint32_t exclusive_scissor_count;
        std::vector<VkRect2D> exclusive_scissors;

        // When the Command Buffer resets, the value most things in this struct don't matter because if they are read without
        // setting the state, it will fail in ValidateDynamicStateIsSet() for us. Some values (ex. the bitset) are tracking in
        // replacement for static_status/dynamic_status so this needs to reset along with those
        void reset() {
            // There are special because the Secondary CB Inheritance is tracking these defaults
            viewport_count = 0u;
            scissor_count = 0u;

            depth_bias_enable = false;

            viewports.clear();
            discard_rectangles.reset();
            color_blend_enable_attachments.reset();
            color_blend_enabled.reset();
            color_blend_equation_attachments.reset();
            color_write_mask_attachments.reset();
            color_blend_advanced_attachments.reset();
            color_write_enabled.reset();
            color_blend_equations.clear();
            color_write_masks.clear();
            vertex_binding_descriptions_divisor.clear();
            vertex_attribute_descriptions.clear();
            viewport_w_scalings.clear();
            exclusive_scissor_enables.clear();
            exclusive_scissors.clear();

            color_write_enable_attachment_count = 0u;
        }
    } dynamic_state_value;

    // Currently storing "lastBound" objects on per-CB basis
    //  long-term may want to create caches of "lastBound" states and could have
    //  each individual CMD_NODE referencing its own "lastBound" state
    // Store last bound state for Gfx & Compute pipeline bind points
    std::array<LastBound, BindPoint_Count> lastBound;  // index is LvlBindPoint.

    // Use the casting boilerplate from StateObject to implement the derived shared_from_this
    std::shared_ptr<const CommandBuffer> shared_from_this() const { return SharedFromThisImpl(this); }
    std::shared_ptr<CommandBuffer> shared_from_this() { return SharedFromThisImpl(this); }

    // If VK_NV_inherited_viewport_scissor is enabled and VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D is
    // true, then is the nonempty list of viewports passed in pViewportDepths. Otherwise, this is empty.
    std::vector<VkViewport> inheritedViewportDepths;

    // For each draw command D recorded to this command buffer, let
    //  * g_D be the graphics pipeline used
    //  * v_G be the viewportCount of g_D (0 if g_D disables rasterization or enables VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT)
    //  * s_G be the scissorCount  of g_D (0 if g_D disables rasterization or enables VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT)
    // Then this value is max(0, max(v_G for all D in cb), max(s_G for all D in cb))
    uint32_t usedViewportScissorCount;
    uint32_t pipelineStaticViewportCount;  // v_G for currently-bound graphics pipeline.
    uint32_t pipelineStaticScissorCount;   // s_G for currently-bound graphics pipeline.

    uint32_t viewportMask;
    uint32_t viewportWithCountMask;
    uint32_t scissorMask;
    uint32_t scissorWithCountMask;

    // Bits set when binding graphics pipeline defining corresponding static state, or executing any secondary command buffer.
    // Bits unset by calling a corresponding vkCmdSet[State] cmd.
    uint32_t trashedViewportMask;
    uint32_t trashedScissorMask;
    bool trashedViewportCount;
    bool trashedScissorCount;

    // True if any draw command recorded to this command buffer consumes dynamic viewport/scissor with count state.
    bool usedDynamicViewportCount;
    bool usedDynamicScissorCount;

    // Track if any dynamic state is set that is static in the currently bound pipeline
    bool dirtyStaticState;

    uint32_t initial_device_mask;

    // The RenderPass created from vkCmdBeginRenderPass or vkCmdBeginRendering
    std::shared_ptr<vvl::RenderPass> activeRenderPass;
    // Used for both type of renderPass
    vvl::unordered_set<uint32_t> active_color_attachments_index;
    uint32_t active_render_pass_device_mask;
    bool has_render_pass_striped;
    uint32_t striped_count;
    // only when not using dynamic rendering
    vku::safe_VkRenderPassBeginInfo active_render_pass_begin_info;
    std::shared_ptr<std::vector<SubpassInfo>> active_subpasses;
    std::shared_ptr<std::vector<vvl::ImageView *>> active_attachments;
    std::set<std::shared_ptr<vvl::ImageView>> attachments_view_states;

    VkSubpassContents activeSubpassContents;
    uint32_t GetActiveSubpass() const { return active_subpass_; }
    void SetActiveSubpass(uint32_t subpass);
    std::optional<VkSampleCountFlagBits> GetActiveSubpassRasterizationSampleCount() const { return active_subpass_sample_count_; }
    void SetActiveSubpassRasterizationSampleCount(VkSampleCountFlagBits rasterization_sample_count) {
        active_subpass_sample_count_ = rasterization_sample_count;
    }
    std::shared_ptr<vvl::Framebuffer> activeFramebuffer;
    // Unified data structs to track objects bound to this command buffer as well as object
    //  dependencies that have been broken : either destroyed objects, or updated descriptor sets
    vvl::unordered_set<std::shared_ptr<StateObject>> object_bindings;
    vvl::unordered_map<VulkanTypedHandle, LogObjectList> broken_bindings;

    QFOTransferBarrierSets<QFOBufferTransferBarrier> qfo_transfer_buffer_barriers;
    QFOTransferBarrierSets<QFOImageTransferBarrier> qfo_transfer_image_barriers;

    // VK_KHR_dynamic_rendering_local_read works like dynamic state, but lives for the rendering lifetime only
    struct RenderingAttachment {
        // VkRenderingAttachmentLocationInfoKHR
        bool set_color_locations = false;
        std::vector<uint32_t> color_locations;
        // VkRenderingInputAttachmentIndexInfoKHR
        bool set_color_indexes = false;
        std::vector<uint32_t> color_indexes;
        const uint32_t *depth_index = nullptr;
        const uint32_t *stencil_index = nullptr;
        void Reset() {
            color_locations.clear();
            color_indexes.clear();
            depth_index = nullptr;
            stencil_index = nullptr;
        }
    } rendering_attachments;

    vvl::unordered_set<VkEvent> waitedEvents;
    std::vector<VkEvent> writeEventsBeforeWait;
    std::vector<VkEvent> events;
    vvl::unordered_set<QueryObject> activeQueries;
    vvl::unordered_set<QueryObject> startedQueries;
    vvl::unordered_set<QueryObject> updatedQueries;
    vvl::unordered_set<QueryObject> renderPassQueries;
    ImageLayoutMap image_layout_map;
    AliasedLayoutMap aliased_image_layout_map;  // storage for potentially aliased images

    vvl::unordered_map<uint32_t, vvl::VertexBufferBinding> current_vertex_buffer_binding_info;
    vvl::IndexBufferBinding index_buffer_binding;

    VkCommandBuffer primaryCommandBuffer;
    // If primary, the secondary command buffers we will call.
    vvl::unordered_set<CommandBuffer *> linkedCommandBuffers;
    // Validation functions run at primary CB queue submit time
    using QueueCallback = std::function<bool(const ValidationStateTracker &device_data, const class vvl::Queue &queue_state,
                                             const CommandBuffer &cb_state)>;
    std::vector<QueueCallback> queue_submit_functions;
    // Used by some layers to defer actions until vkCmdEndRenderPass time.
    // Layers using this are responsible for inserting the callbacks into queue_submit_functions.
    std::vector<QueueCallback> queue_submit_functions_after_render_pass;
    // Validation functions run when secondary CB is executed in primary
    std::vector<std::function<bool(const CommandBuffer &secondary, const CommandBuffer *primary, const vvl::Framebuffer *)>>
        cmd_execute_commands_functions;

    using EventCallback = std::function<bool(CommandBuffer &cb_state, bool do_validate, EventToStageMap &local_event_signal_info,
                                             VkQueue waiting_queue, const Location &loc)>;
    std::vector<EventCallback> eventUpdates;

    std::vector<std::function<bool(CommandBuffer &cb_state, bool do_validate, VkQueryPool &firstPerfQueryPool,
                                   uint32_t perfQueryPass, QueryMap *localQueryToStateMap)>>
        queryUpdates;
    bool performance_lock_acquired = false;
    bool performance_lock_released = false;

    // Cache of current insert label...
    LoggingLabel debug_label;

    std::vector<uint8_t> push_constant_data;
    PushConstantRangesId push_constant_data_ranges;

    // Used for Best Practices tracking
    uint32_t small_indexed_draw_call_count;

    // Video coding related state tracking
    std::shared_ptr<vvl::VideoSession> bound_video_session;
    std::shared_ptr<vvl::VideoSessionParameters> bound_video_session_parameters;
    BoundVideoPictureResources bound_video_picture_resources;
    VideoEncodeRateControlState video_encode_rate_control_state{};
    std::optional<uint32_t> video_encode_quality_level{};
    VideoSessionUpdateMap video_session_updates;

    bool transform_feedback_active{false};
    uint32_t transform_feedback_buffers_bound;

    bool conditional_rendering_active{false};
    bool conditional_rendering_inside_render_pass{false};
    uint32_t conditional_rendering_subpass{0};
    std::vector<VkDescriptorBufferBindingInfoEXT> descriptor_buffer_binding_info;

    mutable std::shared_mutex lock;
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock); }

    CommandBuffer(ValidationStateTracker &dev, VkCommandBuffer handle, const VkCommandBufferAllocateInfo *pAllocateInfo,
                  const vvl::CommandPool *cmd_pool);

    virtual ~CommandBuffer() { Destroy(); }

    void Destroy() override;

    VkCommandBuffer VkHandle() const { return handle_.Cast<VkCommandBuffer>(); }

    vvl::ImageView *GetActiveAttachmentImageViewState(uint32_t index);
    const vvl::ImageView *GetActiveAttachmentImageViewState(uint32_t index) const;

    void AddChild(std::shared_ptr<StateObject> &state_object);
    template <typename T>
    void AddChild(std::shared_ptr<T> &child_node) {
        auto base = std::static_pointer_cast<StateObject>(child_node);
        AddChild(base);
    }

    void RemoveChild(std::shared_ptr<StateObject> &state_object);
    template <typename T>
    void RemoveChild(std::shared_ptr<T> &child_node) {
        auto base = std::static_pointer_cast<StateObject>(child_node);
        RemoveChild(base);
    }

    virtual void Reset();

    void IncrementResources();

    void ResetPushConstantDataIfIncompatible(const vvl::PipelineLayout *pipeline_layout_state);

    std::shared_ptr<const ImageSubresourceLayoutMap> GetImageSubresourceLayoutMap(VkImage image) const;
    std::shared_ptr<ImageSubresourceLayoutMap> GetImageSubresourceLayoutMap(const vvl::Image &image_state);
    const ImageLayoutMap &GetImageSubresourceLayoutMap() const;

    const QFOTransferBarrierSets<QFOImageTransferBarrier> &GetQFOBarrierSets(const QFOImageTransferBarrier &type_tag) const {
        return qfo_transfer_image_barriers;
    }

    const QFOTransferBarrierSets<QFOBufferTransferBarrier> &GetQFOBarrierSets(const QFOBufferTransferBarrier &type_tag) const {
        return qfo_transfer_buffer_barriers;
    }

    // Used to get error message objects, but overloads depending on what information is known
    LogObjectList GetObjectList(VkShaderStageFlagBits stage) const;
    LogObjectList GetObjectList(VkPipelineBindPoint pipeline_bind_point) const;

    vvl::Pipeline *GetCurrentPipeline(VkPipelineBindPoint pipelineBindPoint) const;
    void GetCurrentPipelineAndDesriptorSets(VkPipelineBindPoint pipelineBindPoint, const vvl::Pipeline **rtn_pipe,
                                            const std::vector<LastBound::PER_SET> **rtn_sets) const;

    VkQueueFlags GetQueueFlags() const { return command_pool->queue_flags; }

    template <typename Barrier>
    inline bool IsReleaseOp(const Barrier &barrier) const {
        return (IsTransferOp(barrier)) && (command_pool->queueFamilyIndex == barrier.srcQueueFamilyIndex);
    }
    template <typename Barrier>
    inline bool IsAcquireOp(const Barrier &barrier) const {
        return (IsTransferOp(barrier)) && (command_pool->queueFamilyIndex == barrier.dstQueueFamilyIndex);
    }

    void Begin(const VkCommandBufferBeginInfo *pBeginInfo);
    void End(VkResult result);

    void BeginQuery(const QueryObject &query_obj);
    void EndQuery(const QueryObject &query_obj);
    void EndQueries(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
    void ResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
    bool UpdatesQuery(const QueryObject &query_obj) const;

    void BeginRenderPass(Func command, const VkRenderPassBeginInfo *pRenderPassBegin, VkSubpassContents contents);
    void NextSubpass(Func command, VkSubpassContents contents);
    void UpdateSubpassAttachments(const vku::safe_VkSubpassDescription2 &subpass, std::vector<SubpassInfo> &subpasses);
    void EndRenderPass(Func command);

    void BeginRendering(Func command, const VkRenderingInfo *pRenderingInfo);
    void EndRendering(Func command);

    void BeginVideoCoding(const VkVideoBeginCodingInfoKHR *pBeginInfo);
    void EndVideoCoding(const VkVideoEndCodingInfoKHR *pEndCodingInfo);
    void ControlVideoCoding(const VkVideoCodingControlInfoKHR *pControlInfo);
    void DecodeVideo(const VkVideoDecodeInfoKHR *pDecodeInfo);
    void EncodeVideo(const VkVideoEncodeInfoKHR *pEncodeInfo);

    void ExecuteCommands(vvl::span<const VkCommandBuffer> secondary_command_buffers);

    void UpdateLastBoundDescriptorSets(VkPipelineBindPoint pipeline_bind_point, const vvl::PipelineLayout &pipeline_layout,
                                       uint32_t first_set, uint32_t set_count, const VkDescriptorSet *pDescriptorSets,
                                       std::shared_ptr<vvl::DescriptorSet> &push_descriptor_set, uint32_t dynamic_offset_count,
                                       const uint32_t *p_dynamic_offsets);

    void UpdateLastBoundDescriptorBuffers(VkPipelineBindPoint pipeline_bind_point, const vvl::PipelineLayout &pipeline_layout,
                                          uint32_t first_set, uint32_t set_count, const uint32_t *buffer_indicies,
                                          const VkDeviceSize *buffer_offsets);

    void PushDescriptorSetState(VkPipelineBindPoint pipelineBindPoint, const vvl::PipelineLayout &pipeline_layout, uint32_t set,
                                uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites);

    void UpdateDrawCmd(Func command);
    void UpdateDispatchCmd(Func command);
    void UpdateTraceRayCmd(Func command);
    void UpdatePipelineState(Func command, const VkPipelineBindPoint bind_point);

    virtual void RecordCmd(Func command);
    void RecordStateCmd(Func command, CBDynamicState dynamic_state);
    void RecordStateCmd(Func command, CBDynamicFlags const &state_bits);
    void RecordTransferCmd(Func command, std::shared_ptr<Bindable> &&buf1, std::shared_ptr<Bindable> &&buf2 = nullptr);
    void RecordSetEvent(Func command, VkEvent event, VkPipelineStageFlags2KHR stageMask);
    void RecordResetEvent(Func command, VkEvent event, VkPipelineStageFlags2KHR stageMask);
    virtual void RecordWaitEvents(Func command, uint32_t eventCount, const VkEvent *pEvents,
                                  VkPipelineStageFlags2KHR src_stage_mask);
    void RecordWriteTimestamp(Func command, VkPipelineStageFlags2KHR pipelineStage, VkQueryPool queryPool, uint32_t slot);

    void RecordBarriers(uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                        const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                        const VkImageMemoryBarrier *pImageMemoryBarriers);
    void RecordBarriers(const VkDependencyInfoKHR &dep_info);

    void SetImageViewLayout(const vvl::ImageView &view_state, VkImageLayout layout, VkImageLayout layoutStencil);
    void SetImageViewInitialLayout(const vvl::ImageView &view_state, VkImageLayout layout);

    void SetImageLayout(const vvl::Image &image_state, const VkImageSubresourceRange &image_subresource_range, VkImageLayout layout,
                        VkImageLayout expected_layout = kInvalidLayout);
    void SetImageLayout(const vvl::Image &image_state, const VkImageSubresourceLayers &image_subresource_layers,
                        VkImageLayout layout);
    void SetImageInitialLayout(VkImage image, const VkImageSubresourceRange &range, VkImageLayout layout);
    void SetImageInitialLayout(const vvl::Image &image_state, const VkImageSubresourceRange &range, VkImageLayout layout);
    void SetImageInitialLayout(const vvl::Image &image_state, const VkImageSubresourceLayers &layers, VkImageLayout layout);

    void Submit(VkQueue queue, uint32_t perf_submit_pass, const Location &loc);
    void Retire(uint32_t perf_submit_pass, const std::function<bool(const QueryObject &)> &is_query_updated_after);

    uint32_t GetDynamicColorAttachmentCount() const;
    uint32_t GetDynamicColorAttachmentImageIndex(uint32_t index) const { return index; }
    uint32_t GetDynamicColorResolveAttachmentImageIndex(uint32_t index) const { return index + GetDynamicColorAttachmentCount(); }
    uint32_t GetDynamicDepthAttachmentImageIndex() const { return 2 * GetDynamicColorAttachmentCount(); }
    uint32_t GetDynamicDepthResolveAttachmentImageIndex() const { return 2 * GetDynamicColorAttachmentCount() + 1; }
    uint32_t GetDynamicStencilAttachmentImageIndex() const { return 2 * GetDynamicColorAttachmentCount() + 2; }
    uint32_t GetDynamicStencilResolveAttachmentImageIndex() const { return 2 * GetDynamicColorAttachmentCount() + 3; }
    bool HasValidDynamicDepthAttachment() const;
    bool HasValidDynamicStencilAttachment() const;
    bool HasExternalFormatResolveAttachment() const;
    bool HasDynamicDualSourceBlend(uint32_t attachmentCount) const;

    inline void BindPipeline(LvlBindPoint bind_point, vvl::Pipeline *pipe_state) {
        lastBound[bind_point].pipeline_state = pipe_state;
    }
    void BindShader(VkShaderStageFlagBits shader_stage, vvl::ShaderObject *shader_object_state);

    bool IsPrimary() const { return allocate_info.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY; }
    bool IsSeconary() const { return allocate_info.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY; }
    void BeginLabel(const char *label_name);
    void EndLabel();
    int LabelStackDepth() const { return label_stack_depth_; }

    struct LabelCommand {
        bool begin = false;      // vkCmdBeginDebugUtilsLabelEXT or vkCmdEndDebugUtilsLabelEXT
        std::string label_name;  // used when begin == true
    };
    const std::vector<LabelCommand> &GetLabelCommands() const { return label_commands_; }

    // Applies label commands to the label_stack: for "begin label" command it pushes
    // a label on the stack, and for the "end label" command it removes the top label.
    static void ReplayLabelCommands(const vvl::span<const LabelCommand> &label_commands, std::vector<std::string> &label_stack);
    // Computes debug region by replaying given commands on top initial label stack.
    static std::string GetDebugRegionName(const std::vector<LabelCommand> &label_commands, uint32_t label_command_index,
                                          const std::vector<std::string> &initial_label_stack = {});

  private:
    void ResetCBState();

    // Keep track of how many CmdBeginDebugUtilsLabelEXT calls have been made without a matching CmdEndDebugUtilsLabelEXT.
    // Negative value for a secondary command buffer indicates invalid state.
    // Negative value for a primary command buffer is allowed. Validation is done at submit time accross all command buffers.
    int label_stack_depth_ = 0;
    // Used during sumbit time validation.
    std::vector<LabelCommand> label_commands_;

    uint32_t active_subpass_;
    // Stores rasterization samples count obtained from the first pipeline with a pMultisampleState in the active subpass,
    // or std::nullopt
    std::optional<VkSampleCountFlagBits> active_subpass_sample_count_;

  protected:
    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;
    void UpdateAttachmentsView(const VkRenderPassBeginInfo *pRenderPassBegin);
    void EnqueueUpdateVideoInlineQueries(const VkVideoInlineQueryInfoKHR &query_info);
    void UnbindResources();
};

// specializations for barriers that cannot do queue family ownership transfers
template <>
inline bool CommandBuffer::IsReleaseOp(const sync_utils::MemoryBarrier &barrier) const {
    return false;
}
template <>
inline bool CommandBuffer::IsReleaseOp(const VkMemoryBarrier &barrier) const {
    return false;
}
template <>
inline bool CommandBuffer::IsReleaseOp(const VkMemoryBarrier2KHR &barrier) const {
    return false;
}
template <>
inline bool CommandBuffer::IsAcquireOp(const sync_utils::MemoryBarrier &barrier) const {
    return false;
}
template <>
inline bool CommandBuffer::IsAcquireOp(const VkMemoryBarrier &barrier) const {
    return false;
}
template <>
inline bool CommandBuffer::IsAcquireOp(const VkMemoryBarrier2KHR &barrier) const {
    return false;
}

}  // namespace vvl
