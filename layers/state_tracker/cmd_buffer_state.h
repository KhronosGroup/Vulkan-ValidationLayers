/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Copyright (C) 2025 Arm Limited.
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
#include "state_tracker/state_object.h"
#include "state_tracker/image_layout_map.h"
#include "state_tracker/pipeline_library_state.h"
#include "state_tracker/video_session_state.h"
#include "state_tracker/last_bound_state.h"
#include "state_tracker/bind_point.h"
#include "state_tracker/query_state.h"
#include "state_tracker/vertex_index_buffer_state.h"
#include "utils/sync_utils.h"
#include "generated/dynamic_state_helper.h"

struct Location;

namespace vvl {
class Bindable;
class Buffer;
class CommandBufferSubState;
class DeviceState;
class Pipeline;
class Framebuffer;
class Queue;
class RenderPass;
class VideoSession;
class VideoSessionParameters;
}  // namespace vvl

enum class CbState {
    New,                // Newly created CB w/o any cmds
    Recording,          // BeginCB has been called on this CB
    Recorded,           // EndCB has been called on this CB
    InvalidComplete,    // had a complete recording, but was since invalidated
    InvalidIncomplete,  // fouled before recording was completed
};

static inline bool IsRecorded(CbState state) { return state == CbState::Recorded || state == CbState::InvalidComplete; }

static inline bool IsRecording(CbState state) { return state == CbState::Recording || state == CbState::InvalidIncomplete; }

// Submit time validation helper. Since CmdBeginRendering is itself an action command,
// if the first detected action or sync command is CmdBeginRendering, it means there are
// no other action commands before it.
static inline bool HasActionOrSyncCommandBeforeBeginRendering(vvl::Func first_action_or_sync_command) {
    return first_action_or_sync_command != vvl::Func::Empty &&
           !IsValueIn(first_action_or_sync_command, {vvl::Func::vkCmdBeginRendering, vvl::Func::vkCmdBeginRenderingKHR});
}

enum class AttachmentSource {
    Empty = 0,
    RenderPass,
    DynamicRendering,
    Inheritance,  // secondary command buffer VkCommandBufferInheritanceInfo
};

struct AttachmentInfo {
    enum class Type {
        Empty = 0,
        Input,
        Color,
        ColorResolve,
        DepthStencil,
        // Dynamic rendering split DepthStencil up
        Depth,
        DepthResolve,
        Stencil,
        StencilResolve,
        // pNext extended attachment types
        FragmentDensityMap,
        FragmentShadingRate,
    };

    vvl::ImageView *image_view;
    Type type;
    VkImageLayout layout;
    // Only for VkRenderPass with VK_KHR_separate_depth_stencil_layouts
    VkImageLayout separate_stencil_layout;
    // When dealing with color attachments, need to know the index for things such as
    // VkPipelineColorBlendStateCreateInfo::pAttachments or vkCmdSetColorBlendEnableEXT
    //
    // This index is tied to a Attachment::Type and will line up to the index in either:
    //   VkRenderingInfo::pColorAttachments
    //   VkSubpassDescription::pColorAttachments
    //   VkSubpassDescription::pInputAttachments
    //   VkSubpassDescription::pResolveAttachments
    uint32_t type_index;

    AttachmentInfo()
        : image_view(nullptr),
          type(Type::Empty),
          layout(VK_IMAGE_LAYOUT_UNDEFINED),
          separate_stencil_layout(VK_IMAGE_LAYOUT_UNDEFINED),
          type_index(0) {}

    bool IsResolve() const { return type == Type::ColorResolve || type == Type::DepthResolve || type == Type::StencilResolve; }
    bool IsInput() const { return type == Type::Input; }
    bool IsColor() const { return type == Type::Color; }
    bool IsDepth() const;
    bool IsStencil() const;
    bool IsDepthOrStencil() const { return type == Type::DepthStencil || type == Type::Depth || type == Type::Stencil; }
    bool IsFragmentDensityMap() const { return type == Type::FragmentDensityMap; }
    bool IsFragmentShadingRate() const { return type == Type::FragmentShadingRate; }

    std::string Describe(const vvl::CommandBuffer &cb_state, uint32_t rp_index) const;
};

struct SubpassInfo {
    bool used;
    VkImageUsageFlagBits usage;
    VkImageAspectFlags aspectMask;

    SubpassInfo() : used(false), usage(VkImageUsageFlagBits(0)), aspectMask(VkImageAspectFlags(0)) {}
};

namespace vvl {

class Event : public StateObject {
  public:
    Event(VkEvent handle, const VkEventCreateInfo *create_info);
    VkEvent VkHandle() const { return handle_.Cast<VkEvent>(); }

    const VkEventCreateFlags flags;

#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_event_export;
#endif  // VK_USE_PLATFORM_METAL_EXT

    // Signaling state.
    // Gets updated at queue submission granularity or when signaled from the host.
    bool signaled = false;
    vku::safe_VkDependencyInfo dependency_info = {};

    // Source stage specified by the "set event" command.
    // Gets updated at queue submission granularity.
    VkPipelineStageFlags2 signal_src_stage_mask = VK_PIPELINE_STAGE_2_NONE;

    // Queue that signaled this event. It's null if event was signaled from the host.
    VkQueue signaling_queue = VK_NULL_HANDLE;
};

// Track command pools and their command buffers
class CommandPool : public StateObject {
  public:
    DeviceState &dev_data;
    const VkCommandPoolCreateFlags createFlags;
    const uint32_t queueFamilyIndex;
    const VkQueueFlags queue_flags;
    const bool unprotected;  // can't be used for protected memory
    // Cmd buffers allocated from this pool
    vvl::unordered_map<VkCommandBuffer, CommandBuffer *> commandBuffers;

    CommandPool(DeviceState &dev, VkCommandPool handle, const VkCommandPoolCreateInfo *create_info, VkQueueFlags flags);
    virtual ~CommandPool() { Destroy(); }

    VkCommandPool VkHandle() const { return handle_.Cast<VkCommandPool>(); }

    void Allocate(const VkCommandBufferAllocateInfo *allocate_info, const VkCommandBuffer *command_buffers);
    void Free(uint32_t count, const VkCommandBuffer *command_buffers);
    void Reset(const Location &loc);

    void Destroy() override;
};

// This struct is not used to store label inserted with vkCmdInsertDebugUtilsLabelEXT
struct LabelCommand {
    bool begin = false;      // vkCmdBeginDebugUtilsLabelEXT or vkCmdEndDebugUtilsLabelEXT
    std::string label_name;  // used when begin == true
};

class CommandBuffer : public RefcountedStateObject, public SubStateManager<CommandBufferSubState> {
    using Func = vvl::Func;

  public:
    using AliasedLayoutMap = vvl::unordered_map<const ImageLayoutMap *, std::shared_ptr<CommandBufferImageLayoutMap>>;

    VkCommandBufferAllocateInfo allocate_info;

    VkCommandBufferUsageFlags begin_info_flags;
    bool has_inheritance;
    vku::safe_VkCommandBufferInheritanceInfo inheritance_info;

    // since command buffers can only be destroyed by their command pool, this does not need to be a shared_ptr
    const vvl::CommandPool *command_pool;
    DeviceState &dev_data;

    const bool unprotected;  // can't be used for protected memory

    CbState state;           // Track cmd buffer update state
    uint64_t command_count;  // Number of commands recorded. Currently only used with VK_KHR_performance_query
    uint64_t submit_count;   // Number of times CB has been submitted
    uint64_t image_layout_change_count;  // The sequence number for changes to image layout (for cached validation)

    // Track status of all vkCmdSet* calls, if 1, means it was set
    struct DynamicStateStatus {
        CBDynamicFlags cb;        // for lifetime of CommandBuffer (invalidated if static pipeline is bound)
        CBDynamicFlags pipeline;  // for lifetime since last bound pipeline

        CBDynamicFlags history;  // for lifetime of CommandBuffer, regardless if invalidated, used for better error messages

        // There is currently only a single non-graphics dynamic state, for now manage manually to save memory
        bool rtx_stack_size_cb;        // for lifetime of CommandBuffer
        bool rtx_stack_size_pipeline;  // for lifetime since last bound pipeline
    } dynamic_state_status;

    // used to mark which pipeline invalidated dynamic state so error message knows
    // Note that index zero is not used due to the enum size being bitset friendly
    VkPipeline invalidated_state_pipe[CB_DYNAMIC_STATE_STATUS_NUM];
    std::string DescribeInvalidatedState(CBDynamicState dynamic_state) const;

    // Return true if the corresponding vkCmdSet* call has occured in the command buffer.
    // Used for calls like vkCmdSetColorBlendEnableEXT where we have both a VU for
    //   - it was called it at all
    //   - it was called for every attachment
    // And this is used to not give double error
    bool IsDynamicStateSet(CBDynamicState state) const { return dynamic_state_status.cb[state]; }

    // These are values that are being set with vkCmdSet* tied to a command buffer
    struct DynamicStateValue {
        // VK_DYNAMIC_STATE_STENCIL_WRITE_MASK
        uint32_t write_mask_front{};
        uint32_t write_mask_back{};
        // VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE
        bool depth_write_enable{};
        // VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE
        bool depth_test_enable{};
        // VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE
        bool depth_bounds_test_enable{};
        // VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE
        bool stencil_test_enable{};
        // VK_DYNAMIC_STATE_STENCIL_OP
        VkStencilOp fail_op_front{};
        VkStencilOp pass_op_front{};
        VkStencilOp depth_fail_op_front{};
        VkStencilOp fail_op_back{};
        VkStencilOp pass_op_back{};
        VkStencilOp depth_fail_op_back{};
        // VK_DYNAMIC_STATE_CULL_MODE
        VkCullModeFlags cull_mode{};
        // VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
        VkPrimitiveTopology primitive_topology{};
        // VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT
        VkSampleLocationsInfoEXT sample_locations_info{};
        // VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT
        bool discard_rectangle_enable{};
        // VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT
        // maxDiscardRectangles is at max 8 on all known implementations currently
        std::bitset<32> discard_rectangles{};
        // VK_DYNAMIC_STATE_POLYGON_MODE_EXT
        VkPolygonMode polygon_mode{};
        // VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT
        VkSampleCountFlagBits rasterization_samples{};
        // VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT
        uint32_t rasterization_stream{};
        // VK_DYNAMIC_STATE_SAMPLE_MASK_EXT
        VkSampleCountFlagBits samples_mask_samples{};
        // VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_KHR
        VkLineRasterizationMode line_rasterization_mode{};
        // VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT
        bool stippled_line_enable{};
        // VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV
        bool coverage_to_color_enable{};
        // VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV
        uint32_t coverage_to_color_location{};
        // VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV
        VkCoverageModulationModeNV coverage_modulation_mode{};
        // VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV
        bool coverage_modulation_table_enable{};
        // VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV
        bool shading_rate_image_enable{};
        // VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE
        bool rasterizer_discard_enable{};
        // VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE
        bool depth_bias_enable{};
        // VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT
        bool depth_clamp_enable{};
        // VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT
        bool alpha_to_coverage_enable{};
        // VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT
        bool alpha_to_one_enable{};
        // VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT
        bool logic_op_enable{};
        // VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR
        VkExtent2D fragment_size{};
        // VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE
        bool primitive_restart_enable{};

        uint32_t color_write_enable_attachment_count{};

        // maxColorAttachments is at max 8 on all known implementations currently
        std::bitset<32> color_blend_enable_attachments{};            // VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT
        std::bitset<32> color_blend_enabled{};                       // VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT
        std::bitset<32> color_blend_equation_attachments{};          // VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT
        std::bitset<32> color_write_mask_attachments{};              // VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT
        std::bitset<32> color_blend_advanced_attachments{};          // VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT
        std::bitset<32> color_write_enabled{};                       // VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT
        std::vector<VkColorBlendEquationEXT> color_blend_equations;  // VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT
        std::vector<VkColorComponentFlags> color_write_masks;        // VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT

        // VK_DYNAMIC_STATE_VERTEX_INPUT_EXT, key is binding number
        vvl::unordered_map<uint32_t, VertexBindingState> vertex_bindings;

        // VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT
        VkConservativeRasterizationModeEXT conservative_rasterization_mode{};
        // VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT
        bool sample_locations_enable{};
        // VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT
        VkImageAspectFlags attachment_feedback_loop_enable{};

        // VK_DYNAMIC_STATE_VIEWPORT
        std::vector<VkViewport> viewports;
        // and VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT
        uint32_t viewport_count{};
        // VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT
        uint32_t scissor_count{};
        // VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV
        std::vector<VkViewportWScalingNV> viewport_w_scalings;
        uint32_t viewport_w_scaling_first{};
        uint32_t viewport_w_scaling_count{};
        // VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE
        bool viewport_w_scaling_enable{};
        // VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV
        uint32_t viewport_swizzle_count{};
        // VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV
        uint32_t shading_rate_palette_count{};
        // VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV
        uint32_t exclusive_scissor_enable_first{};
        uint32_t exclusive_scissor_enable_count{};
        std::vector<VkBool32> exclusive_scissor_enables{};
        // VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV
        uint32_t exclusive_scissor_first{};
        uint32_t exclusive_scissor_count{};
        std::vector<VkRect2D> exclusive_scissors;

        // When the Command Buffer resets, the value most things in this struct don't matter because if they are read without
        // setting the state, it will fail in ValidateDynamicStateIsSet() for us. Some values (ex. the bitset) are tracking in
        // replacement for static_status/dynamic_status so this needs to reset along with those.
        //
        // The only time this is reset is when the command buffer is reset, and vkCmdBindPipeline for static state
        void reset(CBDynamicFlags mask) {
            // Mask tells which things to reset
            if (mask[CB_DYNAMIC_STATE_VIEWPORT]) {
                viewports.clear();
            }
            if (mask[CB_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT]) {
                discard_rectangles.reset();
            }
            if (mask[CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT]) {
                color_blend_enable_attachments.reset();
                color_blend_enabled.reset();
            }
            if (mask[CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT]) {
                color_blend_equation_attachments.reset();
                color_blend_equations.clear();
            }
            if (mask[CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT]) {
                color_write_mask_attachments.reset();
                color_write_masks.clear();
            }
            if (mask[CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT]) {
                color_blend_advanced_attachments.reset();
            }
            if (mask[CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT]) {
                color_write_enabled.reset();
                color_write_enable_attachment_count = 0u;
            }
            if (mask[CB_DYNAMIC_STATE_VERTEX_INPUT_EXT]) {
                vertex_bindings.clear();
            }
            if (mask[CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV]) {
                viewport_w_scalings.clear();
            }
            if (mask[CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV]) {
                exclusive_scissor_enables.clear();
            }
            if (mask[CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV]) {
                exclusive_scissors.clear();
            }

            // There are special because the Secondary CB Inheritance is tracking these defaults
            if (mask[CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT]) {
                viewport_count = 0u;
            }
            if (mask[CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT]) {
                scissor_count = 0u;
            }
        }
    } dynamic_state_value;

    // Currently storing "lastBound" objects on per-CB basis
    //  long-term may want to create caches of "lastBound" states and could have
    //  each individual CMD_NODE referencing its own "lastBound" state
    // Store last bound state for Gfx & Compute pipeline bind points
    std::array<LastBound, vvl::BindPointCount> lastBound;
    const LastBound &GetLastBoundGraphics() const { return lastBound[vvl::BindPointGraphics]; }
    const LastBound &GetLastBoundCompute() const { return lastBound[vvl::BindPointCompute]; }
    const LastBound &GetLastBoundRayTracing() const { return lastBound[vvl::BindPointRayTracing]; }
    const LastBound &GetLastBoundDataGraph() const { return lastBound[vvl::BindPointDataGraph]; }

    // Use the casting boilerplate from StateObject to implement the derived shared_from_this
    std::shared_ptr<const CommandBuffer> shared_from_this() const { return SharedFromThisImpl(this); }
    std::shared_ptr<CommandBuffer> shared_from_this() { return SharedFromThisImpl(this); }

    // Track if any dynamic state is set that is static in the currently bound pipeline
    bool dirty_static_state;

    // Device Mask at start of command buffer
    uint32_t initial_device_mask;
    // Device mask from vkCmdBeginRenderPass/vkCmdBeginRendering
    uint32_t render_pass_device_mask;

    // Set to true when the first render pass instance is encountered during recording.
    // When recording is finished, it indicates if command buffer has render pass instances.
    bool has_render_pass_instance;

    // True if the *first* render pass instance specifies VK_RENDERING_RESUMING_BIT
    bool resumes_render_pass_instance;

    // The suspension state at the end of the command buffer, based on previous render pass instances.
    // Regular render pass instances (without RESUMING/SUSPENDING) do not change the suspend state.
    enum class SuspendState { Empty, Suspended, Resumed };
    SuspendState last_suspend_state;

    // Used by submit time validation to check for invalild commands when render pass instance is suspended.
    vvl::Func first_action_or_sync_command;

    // This is null if we are outside a renderPass/rendering
    //
    // There are 4 ways we populate this pointer
    // 1. vkCmdBeginRenderPass this becomes a reference to the state created a vkCreateRenderPass time.
    // 2. VkCommandBufferInheritanceInfo same as (1) but for secondary command buffers.
    // 3. vkCmdBeginRendering we create the state object and store it here.
    // 4. VkCommandBufferInheritanceRenderingInfo same as (3) but for secondary command buffers.
    std::shared_ptr<vvl::RenderPass> active_render_pass;

    // Used for both type of renderPass
    AttachmentSource attachment_source;
    // There is no concept of "attachment index" with dynamic rendering, we use this for both dynamic/non-dynamic rendering though.
    // The attachments are packed the following: | color | color resolve | depth | depth resolve | stencil | stencil resolve |
    std::vector<AttachmentInfo> active_attachments;
    // Used for checking all color attachments, values will be [0, colorAttachmentCount - 1] from either VkRenderingInfo or the
    // current subpass. The "active" part means the imageView was not VK_NULL_HANDLE/VK_ATTACHMENT_UNUSED
    vvl::unordered_set<uint32_t> active_color_attachments_index;
    bool has_render_pass_striped;
    uint32_t striped_count;
    VkRect2D render_area;
    // only when not using dynamic rendering
    vku::safe_VkRenderPassSampleLocationsBeginInfoEXT sample_locations_begin_info;
    std::vector<SubpassInfo> active_subpasses;
    const char *DescribeActiveColorAttachment() const;

    VkSubpassContents active_subpass_contents;
    uint32_t GetActiveSubpass() const { return active_subpass_; }
    void SetActiveSubpass(uint32_t subpass);
    std::optional<VkSampleCountFlagBits> GetActiveSubpassRasterizationSampleCount() const { return active_subpass_sample_count_; }
    void SetActiveSubpassRasterizationSampleCount(VkSampleCountFlagBits rasterization_sample_count) {
        active_subpass_sample_count_ = rasterization_sample_count;
    }
    std::shared_ptr<vvl::Framebuffer> active_framebuffer;
    // Unified data structs to track objects bound to this command buffer as well as object
    //  dependencies that have been broken : either destroyed objects, or updated descriptor sets
    vvl::unordered_set<std::shared_ptr<StateObject>> object_bindings;
    vvl::unordered_map<VulkanTypedHandle, LogObjectList> broken_bindings;

    std::vector<TensorBarrier> tensor_barriers;

    // VK_KHR_dynamic_rendering_local_read works like dynamic state, but lives for the rendering lifetime only
    struct RenderingAttachment {
        // VkRenderingAttachmentLocationInfo
        bool set_color_locations = false;
        std::vector<uint32_t> color_locations;
        // VkRenderingInputAttachmentIndexInfo
        bool set_color_indexes = false;
        std::vector<uint32_t> color_indexes;
        const uint32_t *depth_index = nullptr;
        const uint32_t *stencil_index = nullptr;
        uint32_t depth_index_storage;
        uint32_t stencil_index_storage;
        void Reset() {
            color_locations.clear();
            color_indexes.clear();
            depth_index = nullptr;
            stencil_index = nullptr;
        }
    } rendering_attachments;

    vvl::unordered_set<VkEvent> waited_events;
    std::vector<VkEvent> write_events_before_wait;
    std::vector<VkEvent> events;
    vvl::unordered_set<QueryObject> active_queries;
    vvl::unordered_set<QueryObject> started_queries;
    vvl::unordered_set<QueryObject> updated_queries;
    vvl::unordered_set<QueryObject> render_pass_queries;
    ImageLayoutRegistry image_layout_registry;
    AliasedLayoutMap aliased_image_layout_map;  // storage for potentially aliased images

    vvl::unordered_map<uint32_t, vvl::VertexBufferBinding> current_vertex_buffer_binding_info;
    vvl::IndexBufferBinding index_buffer_binding;

    VkCommandBuffer primary_command_buffer;
    // If primary, the secondary command buffers we will call.
    vvl::unordered_set<CommandBuffer *> linked_command_buffers;

    bool performance_lock_acquired = false;
    bool performance_lock_released = false;

    PushConstantRangesId push_constant_ranges_layout;

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

    // VK_EXT_descriptor_buffer
    struct DescriptorBuffer {
        struct BindingInfo {
            VkDeviceAddress address;
            VkBufferUsageFlags2 usage;
        };

        // This information is always tracked
        std::vector<BindingInfo> binding_info;
        bool ever_bound{false};

        void Reset() {
            binding_info.clear();
            ever_bound = false;
        }
    } descriptor_buffer;

    mutable std::shared_mutex lock;
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock); }

    CommandBuffer(DeviceState &dev, VkCommandBuffer handle, const VkCommandBufferAllocateInfo *allocate_info,
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

    void Reset(const Location &loc);

    std::shared_ptr<const CommandBufferImageLayoutMap> GetImageLayoutMap(VkImage image) const;
    std::shared_ptr<CommandBufferImageLayoutMap> GetOrCreateImageLayoutMap(const vvl::Image &image_state);

    // Used to get error message objects, but overloads depending on what information is known
    LogObjectList GetObjectList(VkShaderStageFlagBits stage) const;
    LogObjectList GetObjectList(VkPipelineBindPoint pipeline_bind_point) const;

    VkQueueFlags GetQueueFlags() const { return command_pool->queue_flags; }

    bool IsReleaseOp(const OwnershipTransferBarrier &barrier) const {
        return (IsOwnershipTransfer(barrier)) && (command_pool->queueFamilyIndex == barrier.srcQueueFamilyIndex);
    }
    bool IsAcquireOp(const OwnershipTransferBarrier &barrier) const {
        return (IsOwnershipTransfer(barrier)) && (command_pool->queueFamilyIndex == barrier.dstQueueFamilyIndex);
    }

    void Begin(const VkCommandBufferBeginInfo *pBeginInfo);
    void End(VkResult result);

    void RecordCommand(const Location &loc);

    void RecordBeginQuery(const QueryObject &query_obj, const Location &loc);
    void RecordEndQuery(const QueryObject &query_obj, const Location &loc);
    void RecordEndQueries(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
    void RecordWriteTimestamp(VkQueryPool queryPool, uint32_t slot, const Location &loc);
    void RecordResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, const Location &loc);
    void RecordCopyQueryPoolResults(VkQueryPool queryPool, VkBuffer dstBuffer, uint32_t firstQuery, uint32_t queryCount,
                                    VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags, const Location &loc);
    void RecordWriteAccelerationStructuresProperties(VkQueryPool queryPool, uint32_t firstQuery,
                                                     uint32_t accelerationStructureCount, const Location &loc);
    bool UpdatesQuery(const QueryObject &query_obj) const;

    void RecordBeginRendering(const VkRenderingInfo &rendering_info, const Location &loc);
    void RecordBeginRenderPass(const VkRenderPassBeginInfo &render_pass_begin, const VkSubpassBeginInfo &subpass_begin_info,
                               const Location &loc);
    void RecordNextSubpass(const VkSubpassBeginInfo &subpass_begin_info, const VkSubpassEndInfo *subpass_end_info,
                           const Location &loc);
    void UpdateSubpassAttachments();
    void RecordEndRendering(const VkRenderingEndInfoEXT *pRenderingEndInfo, const Location &loc);
    void RecordEndRenderPass(const VkSubpassEndInfo *subpass_end_info, const Location &loc);

    void RecordBeginVideoCoding(const VkVideoBeginCodingInfoKHR &begin_info, const Location &loc);
    void RecordEndVideoCoding(const Location &loc);
    void RecordControlVideoCoding(const VkVideoCodingControlInfoKHR &control_info, const Location &loc);
    void RecordDecodeVideo(const VkVideoDecodeInfoKHR &decode_info, const Location &loc);
    void RecordEncodeVideo(const VkVideoEncodeInfoKHR &encode_info, const Location &loc);

    void RecordExecuteCommands(vvl::span<const VkCommandBuffer> secondary_command_buffers, const Location &loc);

    void UpdateLastBoundDescriptorSets(VkPipelineBindPoint pipeline_bind_point,
                                       std::shared_ptr<const vvl::PipelineLayout> pipeline_layout, uint32_t first_set,
                                       uint32_t set_count, const VkDescriptorSet *pDescriptorSets,
                                       std::shared_ptr<vvl::DescriptorSet> &push_descriptor_set, uint32_t dynamic_offset_count,
                                       const uint32_t *p_dynamic_offsets, const Location &loc);

    void UpdateLastBoundDescriptorBuffers(VkPipelineBindPoint pipeline_bind_point,
                                          std::shared_ptr<const vvl::PipelineLayout> pipeline_layout, uint32_t first_set,
                                          uint32_t set_count, const uint32_t *buffer_indicies, const VkDeviceSize *buffer_offsets);

    void PushDescriptorSetState(VkPipelineBindPoint pipelineBindPoint, std::shared_ptr<const vvl::PipelineLayout> pipeline_layout,
                                uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites,
                                const Location &loc);

    void RecordDraw(const Location &loc);
    void RecordDispatch(const Location &loc);
    void RecordTraceRay(const Location &loc);

    void RecordStateCmd(CBDynamicState dynamic_state);
    void RecordDynamicState(CBDynamicState dynamic_state);
    void RecordSetViewport(uint32_t first_viewport, uint32_t viewport_count, const VkViewport *viewports);
    void RecordSetViewportWithCount(uint32_t viewport_count, const VkViewport *viewports);
    void RecordSetScissor(uint32_t first_scissor, uint32_t scissor_count);
    void RecordSetScissorWithCount(uint32_t scissor_count);
    void RecordSetDepthCompareOp(VkCompareOp depth_compare_op);
    void RecordSetDepthTestEnable(VkBool32 depth_test_enable);
    void RecordBindPipeline(VkPipelineBindPoint bind_point, vvl::Pipeline &pipeline);

    void RecordCopyBuffer(vvl::Buffer &src_buffer_state, vvl::Buffer &dst_buffer_state, uint32_t region_count,
                          const VkBufferCopy *regions, const Location &loc);
    void RecordCopyBuffer2(vvl::Buffer &src_buffer_state, vvl::Buffer &dst_buffer_state, uint32_t region_count,
                           const VkBufferCopy2 *regions, const Location &loc);
    void RecordCopyImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                         VkImageLayout dst_image_layout, uint32_t region_count, const VkImageCopy *regions, const Location &loc);
    void RecordCopyImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                          VkImageLayout dst_image_layout, uint32_t region_count, const VkImageCopy2 *regions, const Location &loc);
    void RecordCopyBufferToImage(vvl::Buffer &src_buffer_state, vvl::Image &dst_image_state, VkImageLayout dst_image_layout,
                                 uint32_t region_count, const VkBufferImageCopy *regions, const Location &loc);
    void RecordCopyBufferToImage2(vvl::Buffer &src_buffer_state, vvl::Image &dst_image_state, VkImageLayout dst_image_layout,
                                  uint32_t region_count, const VkBufferImageCopy2 *regions, const Location &loc);
    void RecordCopyImageToBuffer(vvl::Image &src_image_state, vvl::Buffer &dst_buffer_state, VkImageLayout src_image_layout,
                                 uint32_t region_count, const VkBufferImageCopy *regions, const Location &loc);
    void RecordCopyImageToBuffer2(vvl::Image &src_image_state, vvl::Buffer &dst_buffer_state, VkImageLayout src_image_layout,
                                  uint32_t region_count, const VkBufferImageCopy2 *regions, const Location &loc);
    void RecordBlitImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                         VkImageLayout dst_image_layout, uint32_t region_count, const VkImageBlit *regions, const Location &loc);
    void RecordBlitImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                          VkImageLayout dst_image_layout, uint32_t region_count, const VkImageBlit2 *regions, const Location &loc);
    void RecordResolveImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, uint32_t region_count,
                            const VkImageResolve *regions, const Location &loc);
    void RecordResolveImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, uint32_t region_count,
                             const VkImageResolve2 *regions, const Location &loc);
    void RecordClearColorImage(vvl::Image &image_state, VkImageLayout image_layout, const VkClearColorValue *color_values,
                               uint32_t range_count, const VkImageSubresourceRange *ranges, const Location &loc);
    void RecordClearDepthStencilImage(vvl::Image &image_state, VkImageLayout image_layout,
                                      const VkClearDepthStencilValue *depth_stencil_values, uint32_t range_count,
                                      const VkImageSubresourceRange *ranges, const Location &loc);
    void RecordClearAttachments(uint32_t attachment_count, const VkClearAttachment *pAttachments, uint32_t rect_count,
                                const VkClearRect *pRects, const Location &loc);
    void RecordFillBuffer(vvl::Buffer &buffer_state, VkDeviceSize offset, VkDeviceSize size, const Location &loc);
    void RecordUpdateBuffer(vvl::Buffer &buffer_state, VkDeviceSize offset, VkDeviceSize size, const Location &loc);

    void RecordSetEvent(VkEvent event, VkPipelineStageFlags2KHR stageMask, const VkDependencyInfo *dependency_info,
                        const Location &loc);
    void RecordResetEvent(VkEvent event, VkPipelineStageFlags2KHR stageMask, const Location &loc);
    void RecordWaitEvents(uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags2KHR src_stage_mask,
                          const VkDependencyInfo *dependency_info, const Location &loc);
    void RecordPushConstants(const vvl::PipelineLayout &pipeline_layout_state, VkShaderStageFlags stage_flags, uint32_t offset,
                             uint32_t size, const void *values);

    void RecordBeginConditionalRendering(const Location &loc);
    void RecordEndConditionalRendering(const Location &loc);

    void RecordSetRenderingAttachmentLocations(const VkRenderingAttachmentLocationInfo *pLocationInfo, const Location &loc);
    void RecordSetRenderingInputAttachmentIndices(const VkRenderingInputAttachmentIndexInfo *pLocationInfo, const Location &loc);

    void RecordBarrierObjects(uint32_t buffer_barrier_count, const VkBufferMemoryBarrier *buffer_barriers,
                              uint32_t image_barrier_count, const VkImageMemoryBarrier *image_barriers,
                              VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask, const Location &loc);
    void RecordBarrierObjects(const VkDependencyInfo &dep_info, const Location &loc);

    void SetImageViewLayout(const vvl::ImageView &view_state, VkImageLayout layout, VkImageLayout layoutStencil);
    void TrackImageViewFirstLayout(const vvl::ImageView &view_state, VkImageLayout layout,
                                   const char *submit_time_layout_mismatch_vuid);

    void SetImageLayout(const vvl::Image &image_state, const VkImageSubresourceRange &normalized_subresource_range,
                        VkImageLayout layout, VkImageLayout expected_layout = kInvalidLayout);

    // This tracks the first known layout of the subresource in the command buffer.
    // NOTE: depth_offset/depth_extent parameters are used to support per-slice image layout
    // transitions in 3d image. Set depth_extent to zero if API does not specify region with depth information.
    void TrackImageFirstLayout(const vvl::Image &image_state, const VkImageSubresourceRange &subresource_range,
                               int32_t depth_offset, uint32_t depth_extent, VkImageLayout layout);

    void SubmitTimeValidate(Queue &queue_state, uint32_t perf_submit_pass, const Location &loc);

    // Helpers to offset into |active_attachments|
    // [all color, all color resolve, depth, depth resolve, stencil, stencil resolve, FragmentDensityMap]
    uint32_t GetDynamicRenderingColorAttachmentCount() const;
    // Used to keep naming convention consistent
    uint32_t GetDynamicRenderingColorAttachmentIndex(uint32_t index) const { return index; }
    uint32_t GetDynamicRenderingColorResolveAttachmentIndex(uint32_t index) const {
        return index + GetDynamicRenderingColorAttachmentCount();
    }
    // used for non-color types
    uint32_t GetDynamicRenderingAttachmentIndex(AttachmentInfo::Type type) const;

    bool HasExternalFormatResolveAttachment() const;

    inline void BindLastBoundPipeline(vvl::BindPoint bind_point, vvl::Pipeline *pipe_state) {
        lastBound[bind_point].pipeline_state = pipe_state;
    }
    void BindShader(VkShaderStageFlagBits shader_stage, vvl::ShaderObject *shader_object_state);

    bool IsPrimary() const { return allocate_info.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY; }
    bool IsSecondary() const { return allocate_info.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY; }

    void BeginLabel(const char *label_name);
    void EndLabel();
    int32_t GetLabelStackDepth() const { return label_stack_depth_; }

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
    int32_t label_stack_depth_ = 0;
    // Used during sumbit time validation.
    std::vector<LabelCommand> label_commands_;

    uint32_t active_subpass_;
    // Stores rasterization samples count obtained from the first pipeline with a pMultisampleState in the active subpass,
    // or std::nullopt
    std::optional<VkSampleCountFlagBits> active_subpass_sample_count_;

  protected:
    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;
    void UpdateAttachmentsView(const VkRenderPassBeginInfo *pRenderPassBegin);
    void RecordVideoInlineQueries(const VkVideoInlineQueryInfoKHR &query_info);
    void UnbindResources();
};

class CommandBufferSubState {
  public:
    explicit CommandBufferSubState(CommandBuffer &cb) : base(cb) {}
    CommandBufferSubState(const CommandBufferSubState &) = delete;
    CommandBufferSubState &operator=(const CommandBufferSubState &) = delete;
    virtual ~CommandBufferSubState() {}

    virtual void Begin(const VkCommandBufferBeginInfo &begin_info) {}
    virtual void End() {}
    virtual void Reset(const Location &loc) {}
    virtual void Destroy() {}

    virtual void RecordExecuteCommand(vvl::CommandBuffer &secondary_command_buffer, uint32_t cmd_index, const Location &loc) {}

    virtual void RecordActionCommand(LastBound &last_bound, const Location &loc) {}
    virtual void RecordBindPipeline(VkPipelineBindPoint bind_point, vvl::Pipeline &pipeline) {}
    virtual void UpdateLastBoundDescriptorSets(VkPipelineBindPoint bind_point, const Location &loc) {}

    virtual void RecordSetViewport(uint32_t first_viewport, uint32_t viewport_count) {}
    virtual void RecordSetViewportWithCount(uint32_t viewport_count) {}
    virtual void RecordSetScissor(uint32_t first_scissor, uint32_t scissor_count) {}
    virtual void RecordSetScissorWithCount(uint32_t scissor_count) {}
    virtual void RecordSetDepthCompareOp(VkCompareOp depth_compare_op) {}
    virtual void RecordSetDepthTestEnable(VkBool32 depth_test_enable) {}

    virtual void RecordCopyBuffer(vvl::Buffer &src_buffer_state, vvl::Buffer &dst_buffer_state, uint32_t region_count,
                                  const VkBufferCopy *regions, const Location &loc) {}
    virtual void RecordCopyBuffer2(vvl::Buffer &src_buffer_state, vvl::Buffer &dst_buffer_state, uint32_t region_count,
                                   const VkBufferCopy2 *regions, const Location &loc) {}
    virtual void RecordCopyImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                                 VkImageLayout dst_image_layout, uint32_t region_count, const VkImageCopy *regions,
                                 const Location &loc) {}
    virtual void RecordCopyImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                                  VkImageLayout dst_image_layout, uint32_t region_count, const VkImageCopy2 *regions,
                                  const Location &loc) {}
    virtual void RecordCopyBufferToImage(vvl::Buffer &src_buffer_state, vvl::Image &dst_image_state, VkImageLayout dst_image_layout,
                                         uint32_t region_count, const VkBufferImageCopy *regions, const Location &loc) {}
    virtual void RecordCopyBufferToImage2(vvl::Buffer &src_buffer_state, vvl::Image &dst_image_state,
                                          VkImageLayout dst_image_layout, uint32_t region_count, const VkBufferImageCopy2 *regions,
                                          const Location &loc) {}
    virtual void RecordCopyImageToBuffer(vvl::Image &src_image_state, vvl::Buffer &dst_buffer_state, VkImageLayout src_image_layout,
                                         uint32_t region_count, const VkBufferImageCopy *regions, const Location &loc) {}
    virtual void RecordCopyImageToBuffer2(vvl::Image &src_image_state, vvl::Buffer &dst_buffer_state,
                                          VkImageLayout src_image_layout, uint32_t region_count, const VkBufferImageCopy2 *regions,
                                          const Location &loc) {}
    virtual void RecordBlitImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                                 VkImageLayout dst_image_layout, uint32_t region_count, const VkImageBlit *regions,
                                 const Location &loc) {}
    virtual void RecordBlitImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, VkImageLayout src_image_layout,
                                  VkImageLayout dst_image_layout, uint32_t region_count, const VkImageBlit2 *regions,
                                  const Location &loc) {}
    virtual void RecordResolveImage(vvl::Image &src_image_state, vvl::Image &dst_image_state, uint32_t region_count,
                                    const VkImageResolve *regions, const Location &loc) {}
    virtual void RecordResolveImage2(vvl::Image &src_image_state, vvl::Image &dst_image_state, uint32_t region_count,
                                     const VkImageResolve2 *regions, const Location &loc) {}
    virtual void RecordClearColorImage(vvl::Image &image_state, VkImageLayout image_layout, const VkClearColorValue *color_values,
                                       uint32_t range_count, const VkImageSubresourceRange *ranges, const Location &loc) {}
    virtual void RecordClearDepthStencilImage(vvl::Image &image_state, VkImageLayout image_layout,
                                              const VkClearDepthStencilValue *depth_stencil_values, uint32_t range_count,
                                              const VkImageSubresourceRange *ranges, const Location &loc) {}
    virtual void RecordClearAttachments(uint32_t attachment_count, const VkClearAttachment *pAttachments, uint32_t rect_count,
                                        const VkClearRect *pRects, const Location &loc) {}
    virtual void RecordFillBuffer(vvl::Buffer &buffer_state, VkDeviceSize offset, VkDeviceSize size, const Location &loc) {}
    virtual void RecordUpdateBuffer(vvl::Buffer &buffer_state, VkDeviceSize offset, VkDeviceSize size, const Location &loc) {}

    virtual void RecordSetEvent(VkEvent event, VkPipelineStageFlags2 stage_mask, const VkDependencyInfo *dependency_info) {}
    virtual void RecordResetEvent(VkEvent event, VkPipelineStageFlags2 stage_mask) {}
    virtual void RecordWaitEvents(uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags2 src_stage_mask,
                                  const VkDependencyInfo *dependency_info, const Location &loc) {}
    virtual void RecordBarriers(uint32_t buffer_barrier_count, const VkBufferMemoryBarrier *buffer_barriers,
                                uint32_t image_barrier_count, const VkImageMemoryBarrier *image_barriers,
                                VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask, const Location &loc) {}
    virtual void RecordBarriers2(const VkDependencyInfo &dep_info, const Location &loc) {}

    virtual void RecordPushConstants(VkPipelineLayout layout, VkShaderStageFlags stage_flags, uint32_t offset, uint32_t size,
                                     const void *values) {}

    virtual void RecordBeginRendering(const VkRenderingInfo &rendering_info, const Location &loc) {}
    virtual void RecordBeginRenderPass(const VkRenderPassBeginInfo &render_pass_begin, const VkSubpassBeginInfo &subpass_begin_info,
                                       const Location &loc) {}
    virtual void RecordNextSubpass(const VkSubpassBeginInfo &subpass_begin_info, const VkSubpassEndInfo *subpass_end_info,
                                   const Location &loc) {}
    // Note - these are called prior to the renderPass object being destroyed
    virtual void RecordEndRendering(const VkRenderingEndInfoEXT *pRenderingEndInfo) {}
    virtual void RecordEndRenderPass(const VkSubpassEndInfo *subpass_end_info, const Location &loc) {}

    virtual void RecordBeginQuery(const QueryObject &query_obj, const Location &loc) {}
    virtual void RecordEndQuery(const QueryObject &query_obj, const Location &loc) {}
    virtual void RecordEndQueries(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {}
    virtual void RecordWriteTimestamp(const QueryObject &query_obj, const Location &loc) {}
    virtual void RecordResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, bool is_perf_query,
                                      const Location &loc) {}
    virtual void RecordCopyQueryPoolResults(vvl::QueryPool &pool_state, vvl::Buffer &dst_buffer_state, uint32_t first_query,
                                            uint32_t query_count, VkDeviceSize dst_offset, VkDeviceSize stride,
                                            VkQueryResultFlags flags, const Location &loc) {}
    virtual void RecordWriteAccelerationStructuresProperties(VkQueryPool queryPool, uint32_t firstQuery,
                                                             uint32_t accelerationStructureCount, const Location &loc) {}
    virtual void RecordVideoInlineQueries(const VkVideoInlineQueryInfoKHR &query_info) {}

    virtual void RecordBeginVideoCoding(vvl::VideoSession &vs_state, const VkVideoBeginCodingInfoKHR &begin_info,
                                        const Location &loc) {}
    virtual void RecordControlVideoCoding(vvl::VideoSession &vs_state, const VkVideoCodingControlInfoKHR &control_info,
                                          const Location &loc) {}
    virtual void RecordDecodeVideo(vvl::VideoSession &vs_state, const VkVideoDecodeInfoKHR &decode_info, const Location &loc) {}
    virtual void RecordEncodeVideo(vvl::VideoSession &vs_state, const VkVideoEncodeInfoKHR &encode_info, const Location &loc) {}

    virtual void ClearPushConstants() {}
    virtual void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) {}

    virtual void Submit(Queue &queue_state, uint32_t perf_submit_pass, const Location &loc) {}

    VulkanTypedHandle Handle() const;
    VkCommandBuffer VkHandle() const;

    CommandBuffer &base;
};

}  // namespace vvl
