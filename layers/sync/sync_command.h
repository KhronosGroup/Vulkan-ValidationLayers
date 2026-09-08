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
#include "containers/custom_containers.h"
#include "containers/span.h"
#include "generated/vk_object_types.h"

struct Location;
struct VulkanTypedHandle;

namespace vvl {
class Buffer;
class DescriptorSet;
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

struct BufferAccessCommand {
    const vvl::Buffer& buffer;
    AccessRange range;
    SyncAccessIndex access_index;
    uint32_t handle_index = vvl::kNoIndex32;
    SyncFlags flags = 0;

    struct Storage {
        AccessRange range;
        uint32_t buffer_index;
        SyncAccessIndex access_index;
        uint32_t handle_index;
        SyncFlags flags;
        BufferAccessCommand MakeCommand(const CommandData& command_data) const;
    };
    Storage MakeStorage(CommandData& command_data) const;
    bool Validate(const CommandBufferContext& cb_context, const Location& loc) const;
    bool Validate(const SyncEnvironment& env, const AccessContext& access_context, const CommandBufferContext& cb_context,
                  ResourceUsageTag replay_tag, const Location& loc) const;
    void Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const;
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

using CommandStorage = std::variant<BufferCopyCommand::Storage, BufferAccessCommand::Storage, ImageCopyCommand::Storage,
                                    BarrierCommand::Storage, BeginRenderPassCommand::Storage, NextSubpassCommand::Storage,
                                    EndRenderPassCommand::Storage, ShaderAccessCommand::Storage>;

struct CommandData {
    std::vector<std::shared_ptr<const vvl::Buffer>> buffers;
    std::vector<std::shared_ptr<const vvl::Image>> images;
    std::vector<std::shared_ptr<const vvl::ImageView>> image_views;
    std::vector<std::shared_ptr<const vvl::RenderPass>> render_passes;
    std::vector<std::shared_ptr<const vvl::Pipeline>> pipelines;
    std::vector<BufferCopyRegion> buffer_copy_regions;
    std::vector<VkImageCopy> image_copy_regions;
    std::vector<BarrierSet> barrier_sets;
    std::vector<ShaderAccessCommand::BufferAccess> descriptor_buffer_accesses;
    std::vector<ShaderAccessCommand::ImageViewAccess> descriptor_image_accesses;

    std::vector<std::shared_ptr<const vvl::DescriptorSet>> descriptor_sets;
    vvl::unordered_set<const vvl::DescriptorSet*> descriptor_set_lookup;

    uint32_t AddBuffer(const vvl::Buffer& buffer);
    uint32_t AddImage(const vvl::Image& image);
    uint32_t AddRenderPass(const vvl::RenderPass& render_pass);
    void AddImageView(const vvl::ImageView& image_view);
    void AddPipeline(const vvl::Pipeline& pipeline);
    void AddDescriptorSet(const vvl::DescriptorSet& descriptor_set);
};

// TODO: CommandEntry won't be needed after all commands are introduced.
// Tag could be derived from command index. Remove entry type when and
// use array of commands instead.
struct CommandEntry {
    ResourceUsageTag tag;
    uint32_t tag_count;
    CommandStorage storage;
};

bool ReplayCommands(SyncEnvironment& env, AccessContext& access_context, const CommandBufferContext& cb_context,
                    ResourceUsageTag base_tag, const Location& loc);

}  // namespace syncval
