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

#include "sync/sync_command.h"
#include "sync/sync_access_context.h"
#include "sync/sync_command_buffer.h"
#include "sync/sync_dynamic_rendering.h"
#include "sync/sync_image.h"
#include "sync/sync_validation.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/render_pass_state.h"
#include "utils/image_utils.h"

namespace syncval {

static const char* GetBufferNamePrefix(BufferName buffer_name) {
    switch (buffer_name) {
        case BufferName::kDstBuffer:
            return "dstBuffer ";
        case BufferName::kIndirect:
            return "indirect ";
        case BufferName::kDrawCount:
            return "draw count ";
        case BufferName::kTransformFeedbackCounter:
            return "transform feedback counter ";
        default:
            assert(false);
            return "";
    }
}

struct CommandReplayContext {
    CommandReplayContext(SyncEnvironment& env, AccessContext& destination_access_context, ResourceUsageTag base_tag)
        : env(env), destination_access_context(destination_access_context), render_pass_instance_offset(uint32_t(base_tag)) {}

    AccessContext& CurrentAccessContext() {
        return render_pass_context ? render_pass_context->CurrentContext() : destination_access_context;
    }
    void BeginRenderPass(const BeginRenderPassCommand& command) {
        render_pass_context.emplace(command.render_pass, command.render_area, env.queue_flags, command.attachment_views,
                                    destination_access_context, command.render_pass_instance_id + render_pass_instance_offset,
                                    env.queue_id);
    }
    void NextSubpass() { render_pass_context->AdvanceSubpass(); }
    void EndRenderPass() { render_pass_context.reset(); }

    SyncEnvironment& env;
    AccessContext& destination_access_context;

    std::vector<ImageRangeGen> rendering_view_gens;
    std::optional<RenderingInstance> rendering_instance;
    uint32_t rendering_instance_id = vvl::kNoIndex32;

    const uint32_t render_pass_instance_offset;
    std::optional<RenderPassAccessContext> render_pass_context;
};

bool ReplayCommands(SyncEnvironment& env, AccessContext& destination_access_context, const CommandBufferContext& cb_context,
                    ResourceUsageTag base_tag, const Location& loc) {
    bool skip = false;
    const CommandData& command_data = cb_context.GetCommandData();
    CommandReplayContext replay_context(env, destination_access_context, base_tag);

    auto replay_common = [&skip, &command_data, &env, &cb_context, base_tag, &loc](
                             const auto& storage, AccessContext& access_context, ResourceUsageTag replay_tag) {
        const auto command = storage.MakeCommand(command_data);
        skip |= command.Validate(env, access_context, cb_context, replay_tag, loc);
        const ResourceUsageTag tag = base_tag + replay_tag;
        command.Apply(env, tag, access_context);
    };

    auto replay_draw = [&skip, &command_data, &replay_context, &env, &cb_context, base_tag, &loc](
                           const auto& storage, AccessContext& access_context, ResourceUsageTag replay_tag) {
        RenderPassAccessContext* render_pass_context =
            replay_context.render_pass_context ? &*replay_context.render_pass_context : nullptr;
        const RenderingInstance* rendering_instance =
            replay_context.rendering_instance ? &*replay_context.rendering_instance : nullptr;

        auto command = storage.MakeCommand(command_data, render_pass_context, rendering_instance);

        // Render pass attachment accesses get their adjusted id from RenderPassAccessContext.
        // Only shader accesses ids need to be adjusted here
        if (command.shader_accesses.render_pass_instance_id != vvl::kNoIndex32) {
            command.shader_accesses.render_pass_instance_id += replay_context.render_pass_instance_offset;
        }

        // BeginRendering already added the replay offset to instance id (that's why no +=)
        // Use it for both shader and attachment accesses
        if (replay_context.rendering_instance) {
            command.shader_accesses.render_pass_instance_id = replay_context.rendering_instance_id;
            command.attachment_accesses.render_pass_instance_id = replay_context.rendering_instance_id;
        }

        skip |= command.Validate(env, access_context, cb_context, replay_tag, loc);
        const ResourceUsageTag tag = base_tag + replay_tag;
        command.Apply(env, tag, access_context);
    };

    for (const CommandEntry& entry : cb_context.GetCommands()) {
        const ResourceUsageTag replay_tag = entry.tag;
        const uint32_t index = entry.command_ref.index;
        AccessContext& access_context = replay_context.CurrentAccessContext();

        switch (entry.command_ref.type) {
            case CommandType::kBufferCopy: {
                replay_common(command_data.buffer_copy_commands[index], access_context, replay_tag);
                continue;
            }
            case CommandType::kBufferAccess: {
                replay_common(command_data.buffer_access_commands[index], access_context, replay_tag);
                continue;
            }
            case CommandType::kImageCopy: {
                replay_common(command_data.image_copy_commands[index], access_context, replay_tag);
                continue;
            }
            case CommandType::kPipelineBarrier: {
                replay_common(command_data.barrier_commands[index], access_context, replay_tag);
                continue;
            }
            case CommandType::kBeginRendering: {
                auto command = command_data.begin_rendering_commands[index].MakeCommand(command_data);
                command.render_pass_instance_id += replay_context.render_pass_instance_offset;
                command.rendering_instance.InitViewGens(replay_context.rendering_view_gens);
                replay_context.rendering_instance = command.rendering_instance;
                replay_context.rendering_instance_id = command.render_pass_instance_id;
                skip |= command.Validate(env, access_context, cb_context, replay_tag, loc);
                command.Apply(env, base_tag + replay_tag, access_context);
                continue;
            }
            case CommandType::kEndRendering: {
                if (!replay_context.rendering_instance) {
                    continue;
                }
                const EndRenderingCommand command{*replay_context.rendering_instance, replay_context.rendering_instance_id};
                skip |= command.Validate(env, access_context, cb_context, replay_tag, loc);
                command.Apply(env, base_tag + replay_tag, access_context);
                replay_context.rendering_instance.reset();
                replay_context.rendering_view_gens.clear();
                continue;
            }
            case CommandType::kBeginRenderPass: {
                const auto command = command_data.begin_render_pass_commands[index].MakeCommand(command_data);
                skip |= command.Validate(env, access_context, cb_context, replay_tag, loc);
                replay_context.BeginRenderPass(command);
                const ResourceUsageTag tag = base_tag + replay_tag;
                command.Apply(env, tag, *replay_context.render_pass_context);
                continue;
            }
            case CommandType::kNextSubpass: {
                const NextSubpassCommand command{};
                skip |= command.Validate(env, *replay_context.render_pass_context, cb_context, replay_tag, loc);
                replay_context.NextSubpass();
                const ResourceUsageTag tag = base_tag + replay_tag;
                command.Apply(env, tag, *replay_context.render_pass_context);
                continue;
            }
            case CommandType::kEndRenderPass: {
                const EndRenderPassCommand command{};
                skip |= command.Validate(env, *replay_context.render_pass_context, cb_context, replay_tag, loc);
                const ResourceUsageTag tag = base_tag + replay_tag;
                command.Apply(env, tag, *replay_context.render_pass_context, destination_access_context);
                replay_context.EndRenderPass();
                continue;
            }
            case CommandType::kShaderAccess: {
                replay_common(command_data.shader_access_commands[index], access_context, replay_tag);
                continue;
            }
            case CommandType::kDispatchIndirect: {
                replay_common(command_data.dispatch_indirect_commands[index], access_context, replay_tag);
                continue;
            }
            case CommandType::kDrawIndirectCount: {
                replay_draw(command_data.draw_indirect_count_commands[index], access_context, replay_tag);
                continue;
            }
            case CommandType::kDrawMeshTasks: {
                replay_draw(command_data.draw_mesh_tasks_commands[index], access_context, replay_tag);
                continue;
            }
        }
        assert(false);
    }
    return skip;
}

void CommandData::Reset() {
    buffer_copy_commands.clear();
    buffer_access_commands.clear();
    image_copy_commands.clear();
    barrier_commands.clear();
    begin_rendering_commands.clear();
    begin_render_pass_commands.clear();
    shader_access_commands.clear();
    dispatch_indirect_commands.clear();
    draw_indirect_count_commands.clear();
    draw_mesh_tasks_commands.clear();

    buffers.clear();
    images.clear();
    image_views.clear();
    render_passes.clear();
    pipelines.clear();
    buffer_copy_regions.clear();
    image_copy_regions.clear();
    barrier_sets.clear();
    rendering_attachments.clear();
    descriptor_buffer_accesses.clear();
    descriptor_image_accesses.clear();

    descriptor_sets.clear();
    descriptor_set_lookup.clear();
}

uint32_t CommandData::AddBuffer(const vvl::Buffer& buffer) {
    const uint32_t index = uint32_t(buffers.size());
    buffers.emplace_back(std::static_pointer_cast<const vvl::Buffer>(buffer.shared_from_this()));
    return index;
}

uint32_t CommandData::AddImage(const vvl::Image& image) {
    const uint32_t index = uint32_t(images.size());
    images.emplace_back(std::static_pointer_cast<const vvl::Image>(image.shared_from_this()));
    return index;
}

uint32_t CommandData::AddRenderPass(const vvl::RenderPass& render_pass) {
    const uint32_t index = uint32_t(render_passes.size());
    render_passes.emplace_back(std::static_pointer_cast<const vvl::RenderPass>(render_pass.shared_from_this()));
    return index;
}

void CommandData::AddImageView(const vvl::ImageView& image_view) {
    image_views.emplace_back(std::static_pointer_cast<const vvl::ImageView>(image_view.shared_from_this()));
}

void CommandData::AddPipeline(const vvl::Pipeline& pipeline) {
    pipelines.emplace_back(std::static_pointer_cast<const vvl::Pipeline>(pipeline.shared_from_this()));
}

void CommandData::AddDescriptorSet(const vvl::DescriptorSet& descriptor_set) {
    if (descriptor_set_lookup.insert(&descriptor_set).second) {
        descriptor_sets.emplace_back(std::static_pointer_cast<const vvl::DescriptorSet>(descriptor_set.shared_from_this()));
    }
}

BufferCopyCommand BufferCopyCommand::Storage::MakeCommand(const CommandData& command_data) const {
    const vvl::Buffer& src_buffer = *command_data.buffers[src_buffer_index];
    const vvl::Buffer& dst_buffer = *command_data.buffers[dst_buffer_index];
    vvl::span<const BufferCopyRegion> regions;
    if (region_count != 0) {
        regions = vvl::make_span(&command_data.buffer_copy_regions[first_region], region_count);
    }
    return {src_buffer, dst_buffer, regions, src_handle_index, dst_handle_index};
}

BufferCopyCommand::Storage BufferCopyCommand::MakeStorage(CommandData& command_data) const {
    const uint32_t src_buffer_index = command_data.AddBuffer(src_buffer);
    const uint32_t dst_buffer_index = command_data.AddBuffer(dst_buffer);

    const uint32_t first_region = uint32_t(command_data.buffer_copy_regions.size());
    const uint32_t region_count = uint32_t(regions.size());
    command_data.buffer_copy_regions.insert(command_data.buffer_copy_regions.end(), regions.begin(), regions.end());

    return {src_buffer_index, dst_buffer_index, first_region, region_count, src_handle_index, dst_handle_index};
}

bool BufferCopyCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return Validate(cb_context.GetSyncEnvironment(), cb_context.GetCbAccessContext(), cb_context, kInvalidTag, loc);
}

bool BufferCopyCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                 const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;

    for (const auto [region_index, region] : vvl::enumerate(regions)) {
        const AccessRange src_range = MakeRange(src_buffer, region.src_offset, region.size);
        auto src_hazard = access_context.DetectHazard(src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
        if (src_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, cb_context, src_buffer.Handle());
            const std::string resource_description = validator.FormatHandle(src_buffer);
            const std::string error = validator.error_messages_.BufferCopyError(
                env, src_hazard, cb_context, replay_tag, loc, resource_description, uint32_t(region_index), src_range);
            skip |= validator.SyncError(src_hazard.Hazard(), objlist, loc, error);
        }
        const AccessRange dst_range = MakeRange(dst_buffer, region.dst_offset, region.size);
        auto dst_hazard = access_context.DetectHazard(dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
        if (dst_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, cb_context, dst_buffer.Handle());
            const std::string resource_description = validator.FormatHandle(dst_buffer);
            const std::string error = validator.error_messages_.BufferCopyError(
                env, dst_hazard, cb_context, replay_tag, loc, resource_description, uint32_t(region_index), dst_range);
            skip |= validator.SyncError(dst_hazard.Hazard(), objlist, loc, error);
        }
        if (skip) {
            break;
        }
    }
    return skip;
}

void BufferCopyCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    const ResourceUsageTagEx src_tag_ex{tag, src_handle_index};
    const ResourceUsageTagEx dst_tag_ex{tag, dst_handle_index};

    for (const BufferCopyRegion& region : regions) {
        const AccessRange src_range = MakeRange(src_buffer, region.src_offset, region.size);
        const AccessRange dst_range = MakeRange(dst_buffer, region.dst_offset, region.size);

        access_context.UpdateAccessState(src_buffer, SYNC_COPY_TRANSFER_READ, src_range, src_tag_ex, 0, env.queue_id);
        access_context.UpdateAccessState(dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range, dst_tag_ex, 0, env.queue_id);
    }
}

BufferAccessCommand BufferAccessCommand::Storage::MakeCommand(const CommandData& command_data) const {
    return {*command_data.buffers[buffer_index], range, access_index, handle_index, flags, buffer_name};
}

BufferAccessCommand::Storage BufferAccessCommand::MakeStorage(CommandData& command_data) const {
    return {range, command_data.AddBuffer(buffer), access_index, handle_index, flags, buffer_name};
}

bool BufferAccessCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    // Buffer markers can execute inside a render pass and need its current subpass context,
    // but in other cases GetCbAccessContext() is sufficient
    const AccessContext& access_context = cb_context.GetCurrentAccessContext();
    return Validate(cb_context.GetSyncEnvironment(), access_context, cb_context, kInvalidTag, loc);
}

bool BufferAccessCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                   const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc) const {
    bool skip = false;
    const bool is_marker = (flags & SyncFlag::kMarker) != 0;
    HazardResult hazard;
    if (is_marker) {
        hazard = access_context.DetectMarkerHazard(buffer, range);
    } else {
        hazard = access_context.DetectHazard(buffer, access_index, range);
    }
    if (!hazard.IsHazard()) {
        return skip;
    }

    const SyncValidator& validator = env.validator;
    LogObjectList objlist;
    if (replay_tag == kInvalidTag && is_marker) {
        objlist.add(buffer.Handle());
    } else {
        objlist = BaseObjectList(env, cb_context, buffer.Handle());
    }
    const char* buffer_name_prefix = GetBufferNamePrefix(buffer_name);
    const std::string resource_description = buffer_name_prefix + validator.FormatHandle(buffer.Handle());
    const std::string error =
        validator.error_messages_.BufferError(env, hazard, cb_context, replay_tag, loc, resource_description, range);
    return validator.SyncError(hazard.Hazard(), objlist, loc, error);
}

void BufferAccessCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    access_context.UpdateAccessState(buffer, access_index, range, ResourceUsageTagEx{tag, handle_index}, flags, env.queue_id);
}

ImageCopyCommand ImageCopyCommand::Storage::MakeCommand(const CommandData& command_data) const {
    const vvl::Image& src_image = *command_data.images[src_image_index];
    const vvl::Image& dst_image = *command_data.images[dst_image_index];
    vvl::span<const VkImageCopy> regions;
    if (region_count != 0) {
        regions = vvl::make_span(&command_data.image_copy_regions[first_region], region_count);
    }
    return {src_image, dst_image, regions, src_handle_index, dst_handle_index};
}

ImageCopyCommand::Storage ImageCopyCommand::MakeStorage(CommandData& command_data) const {
    const uint32_t src_image_index = command_data.AddImage(src_image);
    const uint32_t dst_image_index = command_data.AddImage(dst_image);

    const uint32_t first_region = uint32_t(command_data.image_copy_regions.size());
    const uint32_t region_count = uint32_t(regions.size());
    command_data.image_copy_regions.insert(command_data.image_copy_regions.end(), regions.begin(), regions.end());

    return {src_image_index, dst_image_index, first_region, region_count, src_handle_index, dst_handle_index};
}

bool ImageCopyCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return Validate(cb_context.GetSyncEnvironment(), cb_context.GetCbAccessContext(), cb_context, kInvalidTag, loc);
}

bool ImageCopyCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;

    for (const auto [region_index, region] : vvl::enumerate(regions)) {
        auto src_hazard = access_context.DetectHazard(src_image, RangeFromLayers(region.srcSubresource), region.srcOffset,
                                                      region.extent, SYNC_COPY_TRANSFER_READ);
        if (src_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, cb_context, src_image.Handle());
            const std::string resource_description = validator.FormatHandle(src_image);
            const std::string error = validator.error_messages_.ImageCopyResolveBlitError(
                env, src_hazard, cb_context, replay_tag, loc, resource_description, uint32_t(region_index), region.srcOffset,
                region.extent, region.srcSubresource);
            skip |= validator.SyncError(src_hazard.Hazard(), objlist, loc, error);
        }
        auto dst_hazard = access_context.DetectHazard(dst_image, RangeFromLayers(region.dstSubresource), region.dstOffset,
                                                      region.extent, SYNC_COPY_TRANSFER_WRITE);
        if (dst_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, cb_context, dst_image.Handle());
            const std::string resource_description = validator.FormatHandle(dst_image);
            const std::string error = validator.error_messages_.ImageCopyResolveBlitError(
                env, dst_hazard, cb_context, replay_tag, loc, resource_description, uint32_t(region_index), region.dstOffset,
                region.extent, region.dstSubresource);
            skip |= validator.SyncError(dst_hazard.Hazard(), objlist, loc, error);
        }
        if (skip) {
            break;
        }
    }
    return skip;
}

static void UpdateImageAccessState(AccessContext& access_context, const vvl::Image& image, SyncAccessIndex current_usage,
                                   const VkImageSubresourceRange& subresource_range, const VkOffset3D& offset,
                                   const VkExtent3D& extent, ResourceUsageTagEx tag_ex, QueueId queue_id) {
    const auto& sub_state = SubState(image);
    ImageRangeGen range_gen = sub_state.MakeImageRangeGen(subresource_range, offset, extent, false);
    access_context.UpdateAccessState(range_gen, current_usage, tag_ex, 0, queue_id);
}

void ImageCopyCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    const ResourceUsageTagEx src_tag_ex{tag, src_handle_index};
    const ResourceUsageTagEx dst_tag_ex{tag, dst_handle_index};

    for (const VkImageCopy& region : regions) {
        UpdateImageAccessState(access_context, src_image, SYNC_COPY_TRANSFER_READ, RangeFromLayers(region.srcSubresource),
                               region.srcOffset, region.extent, src_tag_ex, env.queue_id);
        UpdateImageAccessState(access_context, dst_image, SYNC_COPY_TRANSFER_WRITE, RangeFromLayers(region.dstSubresource),
                               region.dstOffset, region.extent, dst_tag_ex, env.queue_id);
    }
}

BarrierCommand BarrierCommand::Storage::MakeCommand(const CommandData& command_data) const {
    return BarrierCommand{command_data.barrier_sets[barrier_set_index]};
}

BarrierCommand::Storage BarrierCommand::MakeStorage(CommandData& command_data) const {
    const uint32_t barrier_set_index = uint32_t(command_data.barrier_sets.size());
    command_data.barrier_sets.emplace_back(barrier_set);
    return {barrier_set_index};
}

bool BarrierCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return Validate(cb_context.GetSyncEnvironment(), cb_context.GetCurrentAccessContext(), cb_context, kInvalidTag, loc);
}

bool BarrierCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                              const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;

    for (const auto& image_barrier : barrier_set.image_barriers) {
        if (!image_barrier.layout_transition) {
            // The only accesses that originate from the pipeline barrier are layout transitions
            continue;
        }
        const vvl::Image& image_state = *image_barrier.image;
        const bool can_transition_depth_slices =
            CanTransitionDepthSlices(validator.extensions, image_state.GetImageType(), image_state.create_flags);

        const auto hazard = access_context.DetectImageBarrierHazard(
            image_state, image_barrier.barrier.src_exec_scope.exec_scope, image_barrier.barrier.src_access_scope,
            image_barrier.subresource_range, can_transition_depth_slices, AccessContext::kDetectAll, env.queue_id);

        if (hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, cb_context, image_state.Handle());
            const std::string resource_description = validator.FormatHandle(image_state.Handle());
            const std::string error = validator.error_messages_.ImageBarrierError(env, hazard, cb_context, replay_tag, loc,
                                                                                  resource_description, image_barrier);
            skip |= validator.SyncError(hazard.Hazard(), objlist, loc, error);
        }
    }
    return skip;
}

void BarrierCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    ApplyBarrier(env, access_context, barrier_set, tag, true);
}

BeginRenderingCommand BeginRenderingCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const RenderingAttachment> attachments;
    if (attachment_count != 0) {
        attachments = vvl::make_span(&command_data.rendering_attachments[first_attachment], attachment_count);
    }
    return {{flags, render_area, view_mask, color_attachment_count, attachments}, render_pass_instance_id};
}

BeginRenderingCommand::Storage BeginRenderingCommand::MakeStorage(CommandData& command_data) const {
    const uint32_t first_attachment = uint32_t(command_data.rendering_attachments.size());
    const uint32_t attachment_count = uint32_t(rendering_instance.attachments.size());
    vvl::Append(command_data.rendering_attachments, rendering_instance.attachments);
    return {rendering_instance.flags,
            rendering_instance.render_area,
            rendering_instance.view_mask,
            rendering_instance.color_attachment_count,
            first_attachment,
            attachment_count,
            render_pass_instance_id};
}

bool BeginRenderingCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return Validate(cb_context.GetSyncEnvironment(), cb_context.GetCbAccessContext(), cb_context, kInvalidTag, loc);
}

bool BeginRenderingCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                     const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                     const Location& loc) const {
    return rendering_instance.ValidateBeginRendering(env, access_context, cb_context, replay_tag, loc, render_pass_instance_id);
}

void BeginRenderingCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    rendering_instance.RecordBeginRendering(access_context, render_pass_instance_id, tag, env.queue_id);
}

bool EndRenderingCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return Validate(cb_context.GetSyncEnvironment(), cb_context.GetCurrentAccessContext(), cb_context, kInvalidTag, loc);
}

bool EndRenderingCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                   const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc) const {
    return rendering_instance.ValidateEndRendering(env, access_context, cb_context, replay_tag, loc, render_pass_instance_id);
}

void EndRenderingCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    rendering_instance.RecordEndRendering(access_context, render_pass_instance_id, tag, env.queue_id);
}

BeginRenderPassCommand BeginRenderPassCommand::Storage::MakeCommand(const CommandData& command_data) const {
    const vvl::RenderPass& render_pass = *command_data.render_passes[render_pass_index];
    vvl::span<const std::shared_ptr<const vvl::ImageView>> attachment_views;
    if (attachment_count != 0) {
        attachment_views = vvl::make_span(&command_data.image_views[first_attachment_view_index], attachment_count);
    }
    return BeginRenderPassCommand{render_pass, attachment_views, render_area, render_pass_instance_id};
}

BeginRenderPassCommand::Storage BeginRenderPassCommand::MakeStorage(CommandData& command_data) const {
    const uint32_t render_pass_index = command_data.AddRenderPass(render_pass);
    const uint32_t first_attachment = uint32_t(command_data.image_views.size());
    const uint32_t attachment_count = uint32_t(attachment_views.size());
    command_data.image_views.insert(command_data.image_views.end(), attachment_views.begin(), attachment_views.end());
    return {render_pass_index, first_attachment, attachment_count, render_area, render_pass_instance_id};
}

bool BeginRenderPassCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return Validate(cb_context.GetSyncEnvironment(), cb_context.GetCbAccessContext(), cb_context, kInvalidTag, loc);
}

bool BeginRenderPassCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                      const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                      const Location& loc) const {
    bool skip = false;
    const uint32_t view_mask = render_pass.create_info.pSubpasses[0].viewMask;

    // Build temp subpass-0 context for simulating initial layout transitions.
    // NOTE: nullptr contexts parameter is safe for subpass zero:
    //  a) its non-external dependencies map is empty (an entry is created
    //     for src_subpass < dst_subpass but dst_subpass is zero)
    //  b) async list for subpass 0 is also empty (needs prev subpass too)
    AccessContext temp_context(env.validator);
    temp_context.InitFrom(0, env.queue_flags, render_pass.subpass_dependency_infos, nullptr, access_context, env.queue_id);

    // Validation runs before the render-pass context exists, so create the attachment view generators locally
    const AttachmentViewGenVector view_gens = RenderPassAccessContext::CreateAttachmentViewGen(render_area, attachment_views);

    skip |= RenderPassAccessContext::ValidateLayoutTransitions(env, temp_context, render_pass, render_pass_instance_id, 0,
                                                               view_mask, view_gens, cb_context, replay_tag, loc);
    if (!skip) {
        // Simulate initial layout transitions in the temporary context before validating load operations
        RenderPassAccessContext::RecordLayoutTransitions(render_pass, 0, view_gens, kInvalidTag, temp_context);

        skip |= RenderPassAccessContext::ValidateLoadOperation(env, temp_context, render_pass, render_pass_instance_id, 0,
                                                               view_mask, view_gens, cb_context, replay_tag, loc);
    }
    return skip;
}

void BeginRenderPassCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, RenderPassAccessContext& rp_context) const {
    const ResourceUsageTag transition_tag = tag;
    const ResourceUsageTag load_op_tag = tag + 1;
    rp_context.RecordBeginRenderPass(transition_tag, load_op_tag, env.queue_id);
}

bool NextSubpassCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    const RenderPassAccessContext* render_pass_context = cb_context.GetCurrentRenderPassContext();
    if (!render_pass_context) {
        return false;
    }
    return Validate(cb_context.GetSyncEnvironment(), *render_pass_context, cb_context, kInvalidTag, loc);
}
bool NextSubpassCommand::Validate(const SyncEnvironment& env, const RenderPassAccessContext& render_pass_context,
                                  const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc) const {
    return render_pass_context.ValidateNextSubpass(env, cb_context, replay_tag, loc);
}

void NextSubpassCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, RenderPassAccessContext& rp_context) const {
    const ResourceUsageTag resolve_tag = tag;
    const ResourceUsageTag store_tag = tag + 1;
    const ResourceUsageTag transition_tag = tag + 2;
    const ResourceUsageTag load_tag = tag + 3;
    rp_context.RecordNextSubpass(resolve_tag, store_tag, transition_tag, load_tag, env.queue_id);
}

bool EndRenderPassCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    const RenderPassAccessContext* render_pass_context = cb_context.GetCurrentRenderPassContext();
    if (!render_pass_context) {
        return false;
    }
    return Validate(cb_context.GetSyncEnvironment(), *render_pass_context, cb_context, kInvalidTag, loc);
}

bool EndRenderPassCommand::Validate(const SyncEnvironment& env, const RenderPassAccessContext& render_pass_context,
                                    const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                    const Location& loc) const {
    return render_pass_context.ValidateEndRenderPass(env, cb_context, replay_tag, loc);
}

void EndRenderPassCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, RenderPassAccessContext& rp_context,
                                 AccessContext& external_context) const {
    const ResourceUsageTag store_tag = tag;
    const ResourceUsageTag transition_tag = tag + 1;
    rp_context.RecordEndRenderPass(external_context, store_tag, transition_tag, env.queue_id);
}

ShaderAccessCommand ShaderAccessCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const BufferAccess> buffer_accesses;
    if (buffer_access_count != 0) {
        buffer_accesses = vvl::make_span(&command_data.descriptor_buffer_accesses[first_buffer_access], buffer_access_count);
    }
    vvl::span<const ImageViewAccess> image_accesses;
    if (image_access_count != 0) {
        image_accesses = vvl::make_span(&command_data.descriptor_image_accesses[first_image_access], image_access_count);
    }
    return {pipeline, buffer_accesses, image_accesses, render_pass_instance_id, subpass};
}

ShaderAccessCommand::Storage ShaderAccessCommand::MakeStorage(CommandData& command_data) const {
    if (pipeline) {
        command_data.AddPipeline(*pipeline);
    }
    for (const BufferAccess& access : buffer_accesses) {
        command_data.AddBuffer(*access.buffer);
        command_data.AddDescriptorSet(*access.info.descriptor_set);
    }
    for (const ImageViewAccess& access : image_accesses) {
        command_data.AddImageView(*access.image_view);
        command_data.AddDescriptorSet(*access.info.descriptor_set);
    }

    const uint32_t first_buffer_access = uint32_t(command_data.descriptor_buffer_accesses.size());
    const uint32_t buffer_access_count = uint32_t(buffer_accesses.size());
    vvl::Append(command_data.descriptor_buffer_accesses, buffer_accesses);

    const uint32_t first_image_access = uint32_t(command_data.descriptor_image_accesses.size());
    const uint32_t image_access_count = uint32_t(image_accesses.size());
    vvl::Append(command_data.descriptor_image_accesses, image_accesses);

    return {pipeline, first_buffer_access, buffer_access_count, first_image_access, image_access_count, render_pass_instance_id,
            subpass};
}

bool ShaderAccessCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return Validate(cb_context.GetSyncEnvironment(), cb_context.GetCurrentAccessContext(), cb_context, kInvalidTag, loc);
}

bool ShaderAccessCommand::ValidateBufferShaderAccess(const SyncEnvironment& env, const AccessContext& access_context,
                                                     const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                                     const Location& loc, const BufferAccess& buffer_access) const {
    HazardResult hazard = access_context.DetectHazard(*buffer_access.buffer, buffer_access.access_index, buffer_access.range);
    if (!hazard.IsHazard()) {
        return false;
    }
    const SyncValidator& validator = env.validator;
    const DescriptorInfo& info = buffer_access.info;

    LogObjectList objlist = BaseObjectList(env, cb_context, info.resource_handle);
    if (info.resource_handle.type == kVulkanObjectTypeAccelerationStructureKHR) {
        objlist.add(buffer_access.buffer->Handle());
    }
    objlist.add(pipeline->Handle());

    const std::string resource_description = validator.FormatHandle(info.resource_handle);

    std::string error;
    if (info.resource_handle.type == kVulkanObjectTypeAccelerationStructureKHR) {
        error = validator.error_messages_.AccelerationStructureDescriptorError(
            env, hazard, cb_context, replay_tag, loc, resource_description, *pipeline, info.set, *info.descriptor_set,
            info.descriptor_type, info.binding, info.array_element, info.stage);
    } else {
        error = validator.error_messages_.BufferDescriptorError(env, hazard, cb_context, replay_tag, loc, resource_description,
                                                                *pipeline, info.set, *info.descriptor_set, info.descriptor_type,
                                                                info.binding, info.array_element, info.stage);
    }
    return validator.SyncError(hazard.Hazard(), objlist, loc, error);
}

bool ShaderAccessCommand::ValidateImageShaderAccess(const SyncEnvironment& env, const AccessContext& access_context,
                                                    const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                                    const Location& loc, const ImageViewAccess& image_access) const {
    HazardResult hazard;
    if (image_access.access_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
        const AttachmentAccess attachment_access{AttachmentAccessType::Access, SyncOrdering::kRaster, render_pass_instance_id,
                                                 subpass};
        ImageRangeGen range_gen = MakeImageRangeGen(*image_access.image_view, image_access.offset, image_access.extent);
        hazard = access_context.DetectAttachmentHazard(range_gen, image_access.access_index, attachment_access, env.queue_id);
    } else {
        hazard = access_context.DetectHazard(*image_access.image_view, image_access.access_index);
    }
    if (!hazard.IsHazard()) {
        return false;
    }
    const SyncValidator& validator = env.validator;
    const DescriptorInfo& info = image_access.info;

    LogObjectList objlist = BaseObjectList(env, cb_context, info.resource_handle);
    objlist.add(pipeline->Handle());

    std::string resource_description = validator.FormatHandle(info.resource_handle);
    if (replay_tag != kInvalidTag && image_access.image_view->image_state) {
        resource_description += " (" + validator.FormatHandle(image_access.image_view->image_state->Handle()) + ")";
    }

    const std::string error = validator.error_messages_.ImageDescriptorError(
        env, hazard, cb_context, replay_tag, loc, resource_description, *pipeline, info.set, *info.descriptor_set,
        info.descriptor_type, info.binding, info.array_element, info.stage, image_access.image_layout);

    return validator.SyncError(hazard.Hazard(), objlist, loc, error);
}

bool ShaderAccessCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                   const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc) const {
    bool skip = false;
    for (const BufferAccess& access : buffer_accesses) {
        skip |= ValidateBufferShaderAccess(env, access_context, cb_context, replay_tag, loc, access);
    }
    for (const ImageViewAccess& access : image_accesses) {
        skip |= ValidateImageShaderAccess(env, access_context, cb_context, replay_tag, loc, access);
    }
    return skip;
}

void ShaderAccessCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    const AttachmentAccess attachment_access{AttachmentAccessType::Access, SyncOrdering::kRaster, render_pass_instance_id, subpass};
    for (const BufferAccess& access : buffer_accesses) {
        const ResourceUsageTagEx tag_ex{tag, access.handle_index};
        access_context.UpdateAccessState(*access.buffer, access.access_index, access.range, tag_ex, 0, env.queue_id);
    }
    for (const ImageViewAccess& access : image_accesses) {
        const ResourceUsageTagEx tag_ex{tag, access.handle_index};
        if (access.access_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
            ImageRangeGen range_gen = MakeImageRangeGen(*access.image_view, access.offset, access.extent);
            access_context.UpdateAttachmentAccessState(range_gen, access.access_index, attachment_access, tag_ex, env.queue_id);
        } else {
            ImageRangeGen range_gen = MakeImageRangeGen(*access.image_view);
            access_context.UpdateAccessState(range_gen, access.access_index, tag_ex, 0, env.queue_id);
        }
    }
}

DispatchIndirectCommand DispatchIndirectCommand::Storage::MakeCommand(const CommandData& command_data) const {
    return {shader_access_storage.MakeCommand(command_data), indirect_access_storage.MakeCommand(command_data)};
}

DispatchIndirectCommand::Storage DispatchIndirectCommand::MakeStorage(CommandData& command_data) const {
    return {shader_accesses.MakeStorage(command_data), indirect_access.MakeStorage(command_data)};
}

bool DispatchIndirectCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return Validate(cb_context.GetSyncEnvironment(), cb_context.GetCurrentAccessContext(), cb_context, kInvalidTag, loc);
}

bool DispatchIndirectCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                       const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                       const Location& loc) const {
    bool skip = false;
    skip |= shader_accesses.Validate(env, access_context, cb_context, replay_tag, loc);
    skip |= indirect_access.Validate(env, access_context, cb_context, replay_tag, loc);
    return skip;
}

void DispatchIndirectCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    shader_accesses.Apply(env, tag, access_context);
    indirect_access.Apply(env, tag, access_context);
}

DrawAttachmentCommand DrawAttachmentCommand::Storage::MakeCommand(RenderPassAccessContext* render_pass_context,
                                                                  const RenderingInstance* rendering_instance) const {
    return {pipeline, render_pass_context, rendering_instance, render_pass_instance_id, depth_write, stencil_write};
}

DrawAttachmentCommand::Storage DrawAttachmentCommand::MakeStorage(CommandData& command_data) const {
    if (pipeline) {
        command_data.AddPipeline(*pipeline);
    }
    return {pipeline, render_pass_instance_id, depth_write, stencil_write};
}

bool DrawAttachmentCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return Validate(cb_context.GetSyncEnvironment(), cb_context.GetCurrentAccessContext(), cb_context, kInvalidTag, loc);
}

bool DrawAttachmentCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                     const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                     const Location& loc) const {
    if (render_pass_context) {
        return render_pass_context->ValidateDrawSubpassAttachment(env, cb_context, replay_tag, loc, pipeline, depth_write,
                                                                  stencil_write);
    } else if (rendering_instance) {
        return rendering_instance->ValidateDrawAttachments(env, access_context, cb_context, replay_tag, loc,
                                                           render_pass_instance_id, pipeline, depth_write, stencil_write);
    }
    return false;
}

void DrawAttachmentCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    if (render_pass_context) {
        render_pass_context->RecordDrawSubpassAttachment(pipeline, depth_write, stencil_write, tag, env.queue_id);
    } else if (rendering_instance) {
        rendering_instance->RecordDrawAttachments(access_context, render_pass_instance_id, pipeline, depth_write, stencil_write,
                                                  tag, env.queue_id);
    }
}

DrawIndirectCountCommand DrawIndirectCountCommand::Storage::MakeCommand(const CommandData& command_data,
                                                                        RenderPassAccessContext* render_pass_context,
                                                                        const RenderingInstance* rendering_instance) const {
    return {shader_access_storage.MakeCommand(command_data),
            attachment_access_storage.MakeCommand(render_pass_context, rendering_instance),
            count_access_storage.MakeCommand(command_data)};
}

DrawIndirectCountCommand::Storage DrawIndirectCountCommand::MakeStorage(CommandData& command_data) const {
    return {shader_accesses.MakeStorage(command_data), attachment_accesses.MakeStorage(command_data),
            count_access.MakeStorage(command_data)};
}

bool DrawIndirectCountCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return Validate(cb_context.GetSyncEnvironment(), cb_context.GetCurrentAccessContext(), cb_context, kInvalidTag, loc);
}

bool DrawIndirectCountCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                        const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                        const Location& loc) const {
    bool skip = false;
    skip |= shader_accesses.Validate(env, access_context, cb_context, replay_tag, loc);
    skip |= attachment_accesses.Validate(env, access_context, cb_context, replay_tag, loc);
    skip |= count_access.Validate(env, access_context, cb_context, replay_tag, loc);
    return skip;
}

void DrawIndirectCountCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    shader_accesses.Apply(env, tag, access_context);
    attachment_accesses.Apply(env, tag, access_context);
    count_access.Apply(env, tag, access_context);
}

DrawMeshTasksCommand DrawMeshTasksCommand::Storage::MakeCommand(const CommandData& command_data,
                                                                RenderPassAccessContext* render_pass_context,
                                                                const RenderingInstance* rendering_instance) const {
    return {shader_access_storage.MakeCommand(command_data),
            attachment_access_storage.MakeCommand(render_pass_context, rendering_instance)};
}

DrawMeshTasksCommand::Storage DrawMeshTasksCommand::MakeStorage(CommandData& command_data) const {
    return {shader_accesses.MakeStorage(command_data), attachment_accesses.MakeStorage(command_data)};
}

bool DrawMeshTasksCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return Validate(cb_context.GetSyncEnvironment(), cb_context.GetCurrentAccessContext(), cb_context, kInvalidTag, loc);
}

bool DrawMeshTasksCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                    const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                    const Location& loc) const {
    bool skip = false;
    skip |= shader_accesses.Validate(env, access_context, cb_context, replay_tag, loc);
    skip |= attachment_accesses.Validate(env, access_context, cb_context, replay_tag, loc);
    return skip;
}

void DrawMeshTasksCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    shader_accesses.Apply(env, tag, access_context);
    attachment_accesses.Apply(env, tag, access_context);
}

}  // namespace syncval
