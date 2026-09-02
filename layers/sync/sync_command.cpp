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
#include "sync/sync_image.h"
#include "sync/sync_validation.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/render_pass_state.h"
#include "utils/image_utils.h"

namespace syncval {

struct CommandReplayContext {
    CommandReplayContext(SyncEnvironment& env, AccessContext& destination_access_context, ResourceUsageTag base_tag)
        : env(env), destination_access_context(destination_access_context), render_pass_instance_offset(uint32_t(base_tag)) {}

    AccessContext& CurrentAccessContext() {
        return render_pass_context ? render_pass_context->CurrentContext() : destination_access_context;
    }
    void BeginRenderPass(const BeginRenderPassCommand& command) {
        render_pass_context.emplace(command.render_pass, command.render_area, env.queue_flags, command.attachment_views,
                                    destination_access_context, command.render_pass_instance_id + render_pass_instance_offset);
    }
    void EndRenderPass() { render_pass_context.reset(); }

    SyncEnvironment& env;
    AccessContext& destination_access_context;
    const uint32_t render_pass_instance_offset;
    std::optional<RenderPassAccessContext> render_pass_context;
};

bool ReplayCommands(SyncEnvironment& env, AccessContext& destination_access_context, const CommandBufferContext& cb_context,
                    ResourceUsageTag base_tag, const Location& loc) {
    bool skip = false;
    const CommandData& command_data = cb_context.GetCommandData();
    CommandReplayContext replay_context(env, destination_access_context, base_tag);

    for (const CommandEntry& entry : cb_context.GetCommands()) {
        const ResourceUsageTag tag = base_tag + entry.tag;
        std::visit(
            [&](const auto& storage) {
                bool command_skip = false;
                const auto& command = storage.MakeCommand(command_data);
                using CommandType = std::decay_t<decltype(command)>;

                AccessContext& access_context = replay_context.CurrentAccessContext();

                if constexpr (std::is_same_v<CommandType, BeginRenderPassCommand>) {
                    command_skip = command.Validate(env, access_context, cb_context, entry.tag, loc);
                    replay_context.BeginRenderPass(command);
                    if (!command_skip) {
                        command.Apply(env, tag, *replay_context.render_pass_context);
                    }
                } else {
                    command_skip = command.Validate(env, access_context, cb_context, entry.tag, loc);
                    if (!command_skip) {
                        command.Apply(env, tag, access_context);
                    }
                }
                skip |= command_skip;
            },
            entry.storage);
    }
    return skip;
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
    temp_context.InitFrom(0, env.queue_flags, render_pass.subpass_dependency_infos, nullptr, access_context);

    // Validation runs before the render-pass context exists, so create the attachment view generators locally
    const AttachmentViewGenVector view_gens = RenderPassAccessContext::CreateAttachmentViewGen(render_area, attachment_views);

    skip |= RenderPassAccessContext::ValidateLayoutTransitions(env, temp_context, render_pass, render_pass_instance_id, 0,
                                                               view_mask, view_gens, cb_context, replay_tag, loc);
    if (!skip) {
        // Simulate initial layout transitions in the temporary context before validating load operations
        RenderPassAccessContext::RecordLayoutTransitions(render_pass, 0, view_gens, kInvalidTag, temp_context, env.queue_id);

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

}  // namespace syncval
