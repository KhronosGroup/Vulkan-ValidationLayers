/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "sync_barrier.h"
#include "sync_dynamic_rendering.h"
#include "containers/custom_containers.h"
#include "containers/small_vector.h"
#include "containers/span.h"
#include "generated/vk_object_types.h"

struct Location;
struct VulkanTypedHandle;

namespace vvl {
class Buffer;
class DescriptorSet;
class Event;
class Image;
class ImageView;
class Pipeline;
class RenderPass;
enum class Func;
}  // namespace vvl

namespace syncval {

class AccessContext;
class CommandBufferContext;
class RenderPassAccessContext;
struct CommandData;
struct SyncEnvironment;

enum class BufferName : uint8_t {
    kDstBuffer,
    kIndirect,
    kDrawCount,
    kTransformFeedbackCounter,
    kRaygenShaderBindingTable,
    kMissShaderBindingTable,
    kHitShaderBindingTable,
    kCallableShaderBindingTable,
};

enum class CommandType : uint32_t {
    kBufferCopy,
    kBufferAccess,
    kImageCopy,
    kBufferImageCopy,
    kImageBlit,
    kImageResolve,
    kImageClear,
    kPipelineBarrier,
    kSetEvent,
    kResetEvent,
    kWaitEvents,
    kBeginRendering,
    kEndRendering,
    kBeginRenderPass,
    kNextSubpass,
    kEndRenderPass,
    kShaderAccess,
    kDispatchIndirect,
    kTraceRays,
    kDraw,
    kDrawMulti,
    kDrawIndirect,
    kDrawIndirectCount,
    kDrawMeshTasks,
    kBuildAccelerationStructures,
    kAccelerationStructureCopy,
    kVideo,
    kClearAttachments,
    kQueryCopy,
};

struct BufferCopyRegion {
    VkDeviceSize src_offset;
    VkDeviceSize dst_offset;
    VkDeviceSize size;
};

struct BufferCopyCommand {
    const vvl::Buffer& src_buffer;
    const vvl::Buffer& dst_buffer;
    vvl::span<const BufferCopyRegion> regions;
    uint32_t src_handle_index = vvl::kNoIndex32;
    uint32_t dst_handle_index = vvl::kNoIndex32;

    struct Storage {
        uint32_t src_buffer_index;
        uint32_t dst_buffer_index;
        uint32_t first_region;
        uint32_t region_count;
        uint32_t src_handle_index;
        uint32_t dst_handle_index;
        BufferCopyCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

// Access to a single buffer range
struct BufferAccessCommand {
    const vvl::Buffer& buffer;
    AccessRange range;
    SyncAccessIndex access_index;
    uint32_t handle_index = vvl::kNoIndex32;
    SyncFlags flags = 0;
    BufferName buffer_name = BufferName::kDstBuffer;

    struct Storage {
        AccessRange range;
        uint32_t buffer_index;
        SyncAccessIndex access_index;
        uint32_t handle_index;
        SyncFlags flags;
        BufferName buffer_name;
        BufferAccessCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

// Accesses to multiple strided buffer ranges
struct StridedBufferAccessCommand {
    const vvl::Buffer& buffer;
    VkDeviceSize offset;
    uint32_t count;
    uint32_t stride;
    uint32_t access_size;
    SyncAccessIndex access_index;
    uint32_t handle_index = vvl::kNoIndex32;
    BufferName buffer_name = BufferName::kDstBuffer;

    struct Storage {
        VkDeviceSize offset;
        uint32_t buffer_index;
        uint32_t count;
        uint32_t stride;
        SyncAccessIndex access_index;
        uint32_t handle_index;
        uint16_t access_size;
        BufferName buffer_name;
        StridedBufferAccessCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;

  private:
    uint32_t GetRangeCount() const;
    AccessRange GetRange(uint32_t index) const;
};

struct ImageCopyCommand {
    const vvl::Image& src_image;
    const vvl::Image& dst_image;
    vvl::span<const VkImageCopy> regions;
    uint32_t src_handle_index = vvl::kNoIndex32;
    uint32_t dst_handle_index = vvl::kNoIndex32;

    struct Storage {
        uint32_t src_image_index;
        uint32_t dst_image_index;
        uint32_t first_region;
        uint32_t region_count;
        uint32_t src_handle_index;
        uint32_t dst_handle_index;
        ImageCopyCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct BufferImageCopyCommand {
    enum class Direction : uint8_t { kBufferToImage, kImageToBuffer };

    const vvl::Buffer& buffer;
    const vvl::Image& image;
    vvl::span<const VkBufferImageCopy> regions;
    Direction direction;
    uint32_t buffer_handle_index = vvl::kNoIndex32;
    uint32_t image_handle_index = vvl::kNoIndex32;

    struct Storage {
        const vvl::Buffer* buffer;
        const vvl::Image* image;
        uint32_t first_region;
        uint32_t region_count;
        Direction direction;
        uint32_t first_handle_index;  // source followed by destination
        BufferImageCopyCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;

    static small_vector<VkBufferImageCopy, 1> MakeRegions(vvl::span<const VkBufferImageCopy2> regions);
};

struct ImageBlitCommand {
    const vvl::Image& src_image;
    const vvl::Image& dst_image;
    vvl::span<const VkImageBlit> regions;
    uint32_t src_handle_index = vvl::kNoIndex32;
    uint32_t dst_handle_index = vvl::kNoIndex32;

    struct Storage {
        const vvl::Image* src_image;
        const vvl::Image* dst_image;
        uint32_t first_region;
        uint32_t region_count;
        uint32_t src_handle_index;
        uint32_t dst_handle_index;
        ImageBlitCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;

    static small_vector<VkImageBlit, 1> MakeRegions(vvl::span<const VkImageBlit2> regions);
};

struct ImageResolveCommand {
    const vvl::Image& src_image;
    const vvl::Image& dst_image;
    vvl::span<const VkImageResolve> regions;
    uint32_t src_handle_index = vvl::kNoIndex32;
    uint32_t dst_handle_index = vvl::kNoIndex32;

    struct Storage {
        const vvl::Image* src_image;
        const vvl::Image* dst_image;
        uint32_t first_region;
        uint32_t region_count;
        uint32_t src_handle_index;
        uint32_t dst_handle_index;
        ImageResolveCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;

    static small_vector<VkImageResolve, 1> MakeRegions(vvl::span<const VkImageResolve2> regions);
};

struct ImageClearCommand {
    const vvl::Image& image;
    vvl::span<const VkImageSubresourceRange> ranges;
    uint32_t handle_index = vvl::kNoIndex32;

    struct Storage {
        const vvl::Image* image;
        uint32_t first_range;
        uint32_t range_count;
        uint32_t handle_index;
        ImageClearCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct BarrierCommand {
    const BarrierSet& barrier_set;

    struct Storage {
        uint32_t barrier_set_index;
        BarrierCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct SetEventCommand {
    const vvl::Event& event;
    SyncExecScope src_exec_scope;
    vvl::Func command;

    struct Storage {
        const vvl::Event* event;
        SyncExecScope src_exec_scope;
        vvl::Func command;
        SetEventCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;

    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct ResetEventCommand {
    const vvl::Event& event;
    SyncExecScope exec_scope;
    vvl::Func command;

    struct Storage {
        const vvl::Event* event;
        SyncExecScope exec_scope;
        vvl::Func command;
        ResetEventCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct WaitEventsCommand {
    vvl::span<const std::shared_ptr<const vvl::Event>> events;
    vvl::span<const BarrierSet> barrier_sets;
    vvl::Func command;

    struct Storage {
        uint32_t first_event;
        uint32_t event_count;
        uint32_t first_barrier_set;
        uint32_t barrier_set_count;
        vvl::Func command;
        WaitEventsCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct BeginRenderingCommand {
    RenderingInstance rendering_instance;
    uint32_t render_pass_instance_id;

    struct Storage {
        VkRenderingFlags flags;
        VkRect2D render_area;
        uint32_t view_mask;
        uint32_t color_attachment_count;
        uint32_t first_attachment;
        uint32_t attachment_count;
        uint32_t render_pass_instance_id;
        BeginRenderingCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct EndRenderingCommand {
    const RenderingInstance& rendering_instance;
    uint32_t render_pass_instance_id;

    struct Storage {};
    Storage MakeStorage(CommandData&) const { return {}; }
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct BeginRenderPassCommand {
    // Initial layout transition and loadOp tags
    static constexpr uint32_t kTagCount = 2;

    const vvl::RenderPass& render_pass;
    vvl::span<const std::shared_ptr<const vvl::ImageView>> attachment_views;
    VkRect2D render_area{};
    uint32_t render_pass_instance_id = 0;

    struct Storage {
        uint32_t render_pass_index;
        uint32_t first_attachment_view_index;
        uint32_t attachment_count;
        VkRect2D render_area;
        uint32_t render_pass_instance_id;
        BeginRenderPassCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, RenderPassAccessContext& rp_context) const;
};

struct NextSubpassCommand {
    // Resolve, store, layout transition, load tags
    static constexpr uint32_t kTagCount = 4;

    struct Storage {
        NextSubpassCommand MakeCommand(const CommandData&) const { return NextSubpassCommand{}; }
    };
    Storage MakeStorage(CommandData&) const { return Storage{}; }
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const RenderPassAccessContext& render_pass_context,
                  const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, RenderPassAccessContext& rp_context) const;
};

struct EndRenderPassCommand {
    // Store/resolve and final layout transition tags
    static constexpr uint32_t kTagCount = 2;

    struct Storage {
        EndRenderPassCommand MakeCommand(const CommandData&) const { return EndRenderPassCommand{}; }
    };
    Storage MakeStorage(CommandData&) const { return Storage{}; }
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const RenderPassAccessContext& render_pass_context,
                  const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, RenderPassAccessContext& rp_context,
               AccessContext& external_context) const;
};

struct ShaderAccessCommand {
    struct DescriptorInfo {
        const vvl::DescriptorSet* descriptor_set;
        VkDescriptorType descriptor_type;
        uint32_t set;
        uint32_t binding;
        uint32_t array_element;
        VkShaderStageFlagBits stage;
        VulkanTypedHandle resource_handle;
    };
    struct BufferAccess {
        DescriptorInfo info;
        const vvl::Buffer* buffer;
        AccessRange range;
        SyncAccessIndex access_index;
        uint32_t handle_index = vvl::kNoIndex32;
    };
    struct ImageViewAccess {
        DescriptorInfo info;
        const vvl::ImageView* image_view;
        VkImageLayout image_layout;
        SyncAccessIndex access_index;
        uint32_t handle_index = vvl::kNoIndex32;
        // Input attachments use the render area and render-pass attachment ordering
        VkOffset3D offset{};
        VkExtent3D extent{};
    };

    const vvl::Pipeline* pipeline = nullptr;
    vvl::span<const BufferAccess> buffer_accesses;
    vvl::span<const ImageViewAccess> image_accesses;
    uint32_t render_pass_instance_id = vvl::kNoIndex32;
    uint32_t subpass = vvl::kNoIndex32;

    struct Storage {
        const vvl::Pipeline* pipeline;
        uint32_t first_buffer_access;
        uint32_t buffer_access_count;
        uint32_t first_image_access;
        uint32_t image_access_count;
        uint32_t render_pass_instance_id;
        uint32_t subpass;
        ShaderAccessCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;

  private:
    bool ValidateBufferShaderAccess(const SyncEnvironment& env, const AccessContext& access_context,
                                    const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc,
                                    const BufferAccess& buffer_access) const;
    bool ValidateImageShaderAccess(const SyncEnvironment& env, const AccessContext& access_context,
                                   const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc,
                                   const ImageViewAccess& image_access) const;
};

// Returned by CommandBufferContext::CollectDescriptorAccesses.
// The same data as ShaderAccessCommand, but owns the buffer/image access arrays
struct DescriptorAccesses {
    const vvl::Pipeline* pipeline = nullptr;
    std::vector<ShaderAccessCommand::BufferAccess> buffer_accesses;
    std::vector<ShaderAccessCommand::ImageViewAccess> image_accesses;
    uint32_t render_pass_instance_id = vvl::kNoIndex32;
    uint32_t subpass = vvl::kNoIndex32;

    // Registers a HandleRecord for each descriptor resource and initializes the
    // handle_index fields of access structures. Call before MakeCommand. Record time only.
    void RegisterResources(CommandBufferContext& cb_context, ResourceUsageTag tag);

    ShaderAccessCommand MakeCommand() const {
        return {pipeline, buffer_accesses, image_accesses, render_pass_instance_id, subpass};
    }
};

struct DispatchIndirectCommand {
    ShaderAccessCommand shader_accesses;
    BufferAccessCommand indirect_access;

    struct Storage {
        ShaderAccessCommand::Storage shader_access_storage;
        BufferAccessCommand::Storage indirect_access_storage;
        DispatchIndirectCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct TraceRaysCommand {
    ShaderAccessCommand shader_accesses;
    vvl::span<const BufferAccessCommand> buffer_accesses;

    struct Storage {
        ShaderAccessCommand::Storage shader_access_storage;
        uint32_t first_buffer_access;
        uint32_t buffer_access_count;
        TraceRaysCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct DrawAttachmentCommand {
    const vvl::Pipeline* pipeline;
    RenderPassAccessContext* render_pass_context;
    const RenderingInstance* rendering_instance;
    uint32_t render_pass_instance_id;
    bool depth_write;
    bool stencil_write;

    struct Storage {
        const vvl::Pipeline* pipeline;
        uint32_t render_pass_instance_id;
        bool depth_write;
        bool stencil_write;
        DrawAttachmentCommand MakeCommand(RenderPassAccessContext* render_pass_context,
                                          const RenderingInstance* rendering_instance) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct VertexInputCommand {
    struct Access {
        const vvl::Buffer* buffer;
        AccessRange range;
        uint32_t handle_index = vvl::kNoIndex32;
    };
    const vvl::Pipeline* pipeline;
    vvl::span<const Access> accesses;
    SyncAccessIndex access_index;

    struct Storage {
        const vvl::Pipeline* pipeline;
        uint32_t first_access;
        uint32_t access_count;
        SyncAccessIndex access_index;
        VertexInputCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct MultiDrawVertexInputCommand {
    struct Binding {
        const vvl::Buffer* buffer;
        VkDeviceSize offset;
        uint32_t stride;
        uint32_t access_size;
        uint32_t handle_index = vvl::kNoIndex32;
    };
    struct DrawRange {
        uint32_t first;
        uint32_t count;
    };

    const vvl::Pipeline* pipeline;
    vvl::span<const Binding> bindings;
    vvl::span<const DrawRange> draws;
    SyncAccessIndex access_index;

    struct Storage {
        const vvl::Pipeline* pipeline;
        uint32_t first_binding;
        uint32_t binding_count;
        uint32_t first_draw;
        uint32_t draw_count;
        SyncAccessIndex access_index;
        MultiDrawVertexInputCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;

  private:
    AccessRange GetRange(const Binding& binding, const DrawRange& draw) const;
};

// Returned by CommandBufferContext::CollectVertexAccesses/CollectIndexAccesses.
// Owns the access array used to construct a VertexInputCommand during recording.
struct VertexInputAccesses {
    const vvl::Pipeline* pipeline = nullptr;
    small_vector<VertexInputCommand::Access, 2> accesses;
    SyncAccessIndex access_index = SYNC_ACCESS_INDEX_NONE;

    void RegisterResources(CommandBufferContext& cb_context, ResourceUsageTag tag);
    VertexInputCommand MakeCommand() const { return {pipeline, accesses, access_index}; }
};

// Returned by CommandBufferContext::CollectMultiDrawVertexAccesses/CollectMultiDrawIndexAccesses.
// Owns the binding and draw arrays used to construct a MultiDrawVertexInputCommand during recording.
struct MultiDrawVertexInputAccesses {
    const vvl::Pipeline* pipeline = nullptr;
    small_vector<MultiDrawVertexInputCommand::Binding, 2> bindings;
    std::vector<MultiDrawVertexInputCommand::DrawRange> draws;
    SyncAccessIndex access_index = SYNC_ACCESS_INDEX_NONE;

    void RegisterResources(CommandBufferContext& cb_context, ResourceUsageTag tag);
    MultiDrawVertexInputCommand MakeCommand() const { return {pipeline, bindings, draws, access_index}; }
};

struct DrawCommand {
    ShaderAccessCommand shader_accesses;
    VertexInputCommand vertex_accesses;
    DrawAttachmentCommand attachment_accesses;

    struct Storage {
        ShaderAccessCommand::Storage shader_access_storage;
        VertexInputCommand::Storage vertex_access_storage;
        DrawAttachmentCommand::Storage attachment_access_storage;
        DrawCommand MakeCommand(const CommandData& command_data, RenderPassAccessContext* render_pass_context,
                                const RenderingInstance* rendering_instance) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct DrawMultiCommand {
    ShaderAccessCommand shader_accesses;
    DrawAttachmentCommand attachment_accesses;
    MultiDrawVertexInputCommand vertex_accesses;

    struct Storage {
        ShaderAccessCommand::Storage shader_access_storage;
        DrawAttachmentCommand::Storage attachment_access_storage;
        MultiDrawVertexInputCommand::Storage vertex_access_storage;
        DrawMultiCommand MakeCommand(const CommandData& command_data, RenderPassAccessContext* render_pass_context,
                                     const RenderingInstance* rendering_instance) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct DrawIndirectCommand {
    ShaderAccessCommand shader_accesses;
    DrawAttachmentCommand attachment_accesses;
    StridedBufferAccessCommand indirect_access;

    struct Storage {
        ShaderAccessCommand::Storage shader_access_storage;
        DrawAttachmentCommand::Storage attachment_access_storage;
        StridedBufferAccessCommand::Storage indirect_access_storage;
        DrawIndirectCommand MakeCommand(const CommandData& command_data, RenderPassAccessContext* render_pass_context,
                                        const RenderingInstance* rendering_instance) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct DrawIndirectCountCommand {
    ShaderAccessCommand shader_accesses;
    DrawAttachmentCommand attachment_accesses;
    BufferAccessCommand count_access;
    // TODO: Track indirect buffer accesses when gpu-av is integrated.
    // indirect buffer accesses depend on runtime count value.

    struct Storage {
        ShaderAccessCommand::Storage shader_access_storage;
        DrawAttachmentCommand::Storage attachment_access_storage;
        BufferAccessCommand::Storage count_access_storage;
        DrawIndirectCountCommand MakeCommand(const CommandData& command_data, RenderPassAccessContext* render_pass_context,
                                             const RenderingInstance* rendering_instance) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct DrawMeshTasksCommand {
    ShaderAccessCommand shader_accesses;
    DrawAttachmentCommand attachment_accesses;

    struct Storage {
        ShaderAccessCommand::Storage shader_access_storage;
        DrawAttachmentCommand::Storage attachment_access_storage;
        DrawMeshTasksCommand MakeCommand(const CommandData& command_data, RenderPassAccessContext* render_pass_context,
                                         const RenderingInstance* rendering_instance) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct BuildAccelerationStructuresCommand {
    enum class AccessType : uint8_t { kScratch, kSource, kDestination, kVertex, kIndex, kTransform, kAABB, kInstance };
    struct Access {
        const vvl::Buffer* buffer;
        AccessRange range;
        AccessType type;
        uint32_t info_index;
        VkAccelerationStructureKHR acceleration_structure = VK_NULL_HANDLE;
        uint32_t handle_index = vvl::kNoIndex32;
    };
    vvl::span<const Access> accesses;

    struct Storage {
        uint32_t first_access;
        uint32_t access_count;
        BuildAccelerationStructuresCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct AccelerationStructureCopyCommand {
    struct Access {
        const vvl::Buffer* buffer;
        AccessRange range;
        VkAccelerationStructureKHR acceleration_structure;
        uint32_t handle_index = vvl::kNoIndex32;
    };
    Access src;
    Access dst;

    struct Storage {
        Access src;
        Access dst;
        AccelerationStructureCopyCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct VideoCommand {
    enum class Operation : uint8_t { kDecode, kEncode };
    enum class PictureType : uint8_t { kOutput, kInput, kReconstructed, kReference, kQuantizationMap };

    struct PictureAccess {
        const vvl::ImageView* view;
        PictureType type;
        uint32_t reference_index;
        // Original VideoPictureResource fields
        VkOffset2D coded_offset;
        VkExtent2D coded_extent;
        uint32_t base_array_layer;
        // Resolved during collection
        VkImageSubresourceRange subresource_range;
        VkOffset2D effective_offset;
        VkExtent2D effective_extent;
    };
    Operation operation;
    const vvl::Buffer& bitstream_buffer;
    AccessRange bitstream_range;
    vvl::span<const PictureAccess> pictures;
    uint32_t bitstream_handle_index = vvl::kNoIndex32;

    struct Storage {
        AccessRange bitstream_range;
        const vvl::Buffer* bitstream_buffer;
        uint32_t first_picture;
        uint32_t picture_count;
        uint32_t bitstream_handle_index;
        Operation operation;
        VideoCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct ClearAttachmentsCommand {
    struct Attachment {
        const vvl::ImageView* view;
        VkImageAspectFlags original_aspects;   // VkClearAttachment::aspectMask
        VkImageAspectFlags effective_aspects;  // aspects actually selected for validation/application
        uint32_t color_attachment;
    };

    vvl::span<const Attachment> attachments;
    vvl::span<const VkClearRect> rects;
    uint32_t view_mask;
    uint32_t render_pass_instance_id;
    uint32_t subpass;

    struct Storage {
        uint32_t first_attachment;
        uint32_t attachment_count;
        uint32_t first_rect;
        uint32_t rect_count;
        uint32_t view_mask;
        uint32_t render_pass_instance_id;
        uint32_t subpass;
        ClearAttachmentsCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct QueryCopyCommand {
    const vvl::Buffer& dst_buffer;
    AccessRange range;
    VkQueryPool query_pool;
    uint32_t handle_index = vvl::kNoIndex32;

    struct Storage {
        const vvl::Buffer* dst_buffer;
        AccessRange range;
        VkQueryPool query_pool;
        uint32_t handle_index;
        QueryCopyCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
};

struct CommandRef {
    CommandType type;
    uint32_t index;
};
static_assert(sizeof(CommandRef) == 8);

struct CommandData {
    //
    // Command storage data
    // NOTE: NextSubpass, EndRenderPass, EndRendering - have no storage data
    //
    std::vector<BufferCopyCommand::Storage> buffer_copy_commands;
    std::vector<BufferAccessCommand::Storage> buffer_access_commands;
    std::vector<ImageCopyCommand::Storage> image_copy_commands;
    std::vector<BufferImageCopyCommand::Storage> buffer_image_copy_commands;
    std::vector<ImageBlitCommand::Storage> image_blit_commands;
    std::vector<ImageResolveCommand::Storage> image_resolve_commands;
    std::vector<ImageClearCommand::Storage> image_clear_commands;
    std::vector<BarrierCommand::Storage> barrier_commands;
    std::vector<SetEventCommand::Storage> set_event_commands;
    std::vector<ResetEventCommand::Storage> reset_event_commands;
    std::vector<WaitEventsCommand::Storage> wait_events_commands;
    std::vector<BeginRenderingCommand::Storage> begin_rendering_commands;
    std::vector<BeginRenderPassCommand::Storage> begin_render_pass_commands;
    std::vector<ShaderAccessCommand::Storage> shader_access_commands;
    std::vector<DispatchIndirectCommand::Storage> dispatch_indirect_commands;
    std::vector<TraceRaysCommand::Storage> trace_rays_commands;
    std::vector<DrawCommand::Storage> draw_commands;
    std::vector<DrawMultiCommand::Storage> draw_multi_commands;
    std::vector<DrawIndirectCommand::Storage> draw_indirect_commands;
    std::vector<DrawIndirectCountCommand::Storage> draw_indirect_count_commands;
    std::vector<DrawMeshTasksCommand::Storage> draw_mesh_tasks_commands;
    std::vector<BuildAccelerationStructuresCommand::Storage> build_acceleration_structures_commands;
    std::vector<AccelerationStructureCopyCommand::Storage> acceleration_structure_copy_commands;
    std::vector<VideoCommand::Storage> video_commands;
    std::vector<ClearAttachmentsCommand::Storage> clear_attachments_commands;
    std::vector<QueryCopyCommand::Storage> query_copy_commands;

    //
    // Resources and additional data used by the commands
    //
    std::vector<std::shared_ptr<const vvl::Buffer>> buffers;
    vvl::unordered_map<const vvl::Buffer*, uint32_t> buffer_lookup;
    const vvl::Buffer* last_buffer = nullptr;  // cache last accessed buffer
    uint32_t last_buffer_index = 0;

    std::vector<std::shared_ptr<const vvl::Image>> images;
    vvl::unordered_map<const vvl::Image*, uint32_t> image_lookup;
    const vvl::Image* last_image = nullptr;  // cache last accessed image
    uint32_t last_image_index = 0;

    std::vector<std::shared_ptr<const vvl::ImageView>> image_views;
    vvl::unordered_set<const vvl::ImageView*> image_view_lookup;
    const vvl::ImageView* last_image_view = nullptr;  // cache last accessed image view

    std::vector<std::shared_ptr<const vvl::Pipeline>> pipelines;
    vvl::unordered_set<const vvl::Pipeline*> pipeline_lookup;
    const vvl::Pipeline* last_pipeline = nullptr;  // cache last accessed pipeline

    std::vector<std::shared_ptr<const vvl::RenderPass>> render_passes;
    std::vector<BufferCopyRegion> buffer_copy_regions;
    std::vector<VkImageCopy> image_copy_regions;
    std::vector<VkBufferImageCopy> buffer_image_copy_regions;
    std::vector<VkImageBlit> image_blit_regions;
    std::vector<VkImageResolve> image_resolve_regions;
    std::vector<VkImageSubresourceRange> image_clear_ranges;
    std::vector<BarrierSet> barrier_sets;
    std::vector<std::shared_ptr<const vvl::Event>> events;
    std::vector<RenderingAttachment> rendering_attachments;
    std::vector<ShaderAccessCommand::BufferAccess> descriptor_buffer_accesses;
    std::vector<ShaderAccessCommand::ImageViewAccess> descriptor_image_accesses;
    std::vector<BufferAccessCommand> trace_rays_buffer_accesses;
    std::vector<VertexInputCommand::Access> vertex_input_accesses;
    std::vector<MultiDrawVertexInputCommand::Binding> multi_draw_vertex_bindings;
    std::vector<MultiDrawVertexInputCommand::DrawRange> multi_draw_ranges;
    std::vector<BuildAccelerationStructuresCommand::Access> acceleration_structure_build_accesses;
    std::vector<VideoCommand::PictureAccess> video_picture_accesses;
    std::vector<ClearAttachmentsCommand::Attachment> clear_attachments;
    std::vector<VkClearRect> clear_rects;

    std::vector<std::shared_ptr<const vvl::DescriptorSet>> descriptor_sets;
    vvl::unordered_set<const vvl::DescriptorSet*> descriptor_set_lookup;

    void Reset();  // keeps capacity

    uint32_t AddBuffer(const vvl::Buffer& buffer);
    uint32_t AddImage(const vvl::Image& image);
    uint32_t AddRenderPass(const vvl::RenderPass& render_pass);
    void AddImageView(const vvl::ImageView& image_view);
    void AddPipeline(const vvl::Pipeline& pipeline);
    void AddDescriptorSet(const vvl::DescriptorSet& descriptor_set);

    CommandRef Store(const BufferCopyCommand::Storage& storage) {
        return Store(CommandType::kBufferCopy, buffer_copy_commands, storage);
    }
    CommandRef Store(const BufferAccessCommand::Storage& storage) {
        return Store(CommandType::kBufferAccess, buffer_access_commands, storage);
    }
    CommandRef Store(const ImageCopyCommand::Storage& storage) {
        return Store(CommandType::kImageCopy, image_copy_commands, storage);
    }
    CommandRef Store(const ImageBlitCommand::Storage& storage) {
        return Store(CommandType::kImageBlit, image_blit_commands, storage);
    }
    CommandRef Store(const ImageResolveCommand::Storage& storage) {
        return Store(CommandType::kImageResolve, image_resolve_commands, storage);
    }
    CommandRef Store(const ImageClearCommand::Storage& storage) {
        return Store(CommandType::kImageClear, image_clear_commands, storage);
    }
    CommandRef Store(const BufferImageCopyCommand::Storage& storage) {
        return Store(CommandType::kBufferImageCopy, buffer_image_copy_commands, storage);
    }
    CommandRef Store(const BarrierCommand::Storage& storage) {
        return Store(CommandType::kPipelineBarrier, barrier_commands, storage);
    }
    CommandRef Store(const SetEventCommand::Storage& storage) { return Store(CommandType::kSetEvent, set_event_commands, storage); }
    CommandRef Store(const ResetEventCommand::Storage& storage) {
        return Store(CommandType::kResetEvent, reset_event_commands, storage);
    }
    CommandRef Store(const WaitEventsCommand::Storage& storage) {
        return Store(CommandType::kWaitEvents, wait_events_commands, storage);
    }
    CommandRef Store(const BeginRenderingCommand::Storage& storage) {
        return Store(CommandType::kBeginRendering, begin_rendering_commands, storage);
    }
    CommandRef Store(const EndRenderingCommand::Storage&) {
        // No storage data, return the command reference directly. The index is unused.
        return {CommandType::kEndRendering, 0};
    }
    CommandRef Store(const BeginRenderPassCommand::Storage& storage) {
        return Store(CommandType::kBeginRenderPass, begin_render_pass_commands, storage);
    }
    CommandRef Store(const NextSubpassCommand::Storage&) {
        // No storage data, return the command reference directly. The index is unused.
        return {CommandType::kNextSubpass, 0};
    }
    CommandRef Store(const EndRenderPassCommand::Storage&) {
        // No storage data, return the command reference directly. The index is unused.
        return {CommandType::kEndRenderPass, 0};
    }
    CommandRef Store(const ShaderAccessCommand::Storage& storage) {
        return Store(CommandType::kShaderAccess, shader_access_commands, storage);
    }
    CommandRef Store(const DispatchIndirectCommand::Storage& storage) {
        return Store(CommandType::kDispatchIndirect, dispatch_indirect_commands, storage);
    }
    CommandRef Store(const TraceRaysCommand::Storage& storage) {
        return Store(CommandType::kTraceRays, trace_rays_commands, storage);
    }
    CommandRef Store(const DrawCommand::Storage& storage) { return Store(CommandType::kDraw, draw_commands, storage); }
    CommandRef Store(const DrawMultiCommand::Storage& storage) {
        return Store(CommandType::kDrawMulti, draw_multi_commands, storage);
    }
    CommandRef Store(const DrawIndirectCommand::Storage& storage) {
        return Store(CommandType::kDrawIndirect, draw_indirect_commands, storage);
    }
    CommandRef Store(const DrawIndirectCountCommand::Storage& storage) {
        return Store(CommandType::kDrawIndirectCount, draw_indirect_count_commands, storage);
    }
    CommandRef Store(const DrawMeshTasksCommand::Storage& storage) {
        return Store(CommandType::kDrawMeshTasks, draw_mesh_tasks_commands, storage);
    }
    CommandRef Store(const BuildAccelerationStructuresCommand::Storage& storage) {
        return Store(CommandType::kBuildAccelerationStructures, build_acceleration_structures_commands, storage);
    }
    CommandRef Store(const AccelerationStructureCopyCommand::Storage& storage) {
        return Store(CommandType::kAccelerationStructureCopy, acceleration_structure_copy_commands, storage);
    }
    CommandRef Store(const VideoCommand::Storage& storage) { return Store(CommandType::kVideo, video_commands, storage); }
    CommandRef Store(const ClearAttachmentsCommand::Storage& storage) {
        return Store(CommandType::kClearAttachments, clear_attachments_commands, storage);
    }
    CommandRef Store(const QueryCopyCommand::Storage& storage) {
        return Store(CommandType::kQueryCopy, query_copy_commands, storage);
    }

  private:
    template <typename Storage>
    static CommandRef Store(CommandType type, std::vector<Storage>& commands, const Storage& storage) {
        const uint32_t index = static_cast<uint32_t>(commands.size());
        commands.push_back(storage);
        return {type, index};
    }
};

// TODO: Revisit tag tracking after command conversion.
// Once tag and tag_count can be derived, store CommandRefs directly
struct CommandEntry {
    CommandRef command_ref;
    ResourceUsageTag tag;
    uint32_t tag_count;
};

bool ReplayCommands(SyncEnvironment& env, AccessContext& access_context, const CommandBufferContext& cb_context,
                    ResourceUsageTag base_tag, const Location& loc);

}  // namespace syncval
