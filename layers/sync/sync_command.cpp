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
#include "sync/sync_event.h"
#include "sync/sync_image.h"
#include "sync/sync_validation.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/event_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/render_pass_state.h"
#include "utils/image_utils.h"
#include "utils/math_utils.h"

#include <algorithm>
#include <cstdlib>

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
        case BufferName::kRaygenShaderBindingTable:
            return "raygen shader binding table ";
        case BufferName::kMissShaderBindingTable:
            return "miss shader binding table ";
        case BufferName::kHitShaderBindingTable:
            return "hit shader binding table ";
        case BufferName::kCallableShaderBindingTable:
            return "callable shader binding table ";
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
                    ResourceUsageTag base_tag, const Location& loc, std::vector<ReportedHazard>* new_hazards) {
    bool skip = false;
    const CommandData& command_data = cb_context.GetCommandData();
    CommandReplayContext replay_context(env, destination_access_context, base_tag);
    ErrorReporter reporter{cb_context, loc, new_hazards, base_tag};

    auto replay_common = [&skip, &command_data, &reporter, &env, base_tag](const auto& storage, AccessContext& access_context) {
        const auto command = storage.MakeCommand(command_data);
        skip |= command.Validate(env, access_context, reporter);
        const ResourceUsageTag tag = base_tag + reporter.replay_tag;
        command.Apply(env, tag, access_context);
    };

    auto replay_draw = [&skip, &command_data, &replay_context, &reporter, &env, base_tag](const auto& storage,
                                                                                          AccessContext& access_context) {
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

        skip |= command.Validate(env, access_context, reporter);
        const ResourceUsageTag tag = base_tag + reporter.replay_tag;
        command.Apply(env, tag, access_context);
    };

    for (const CommandEntry& entry : cb_context.GetCommands()) {
        reporter.replay_tag = entry.tag;
        const uint32_t index = entry.command_ref.index;
        AccessContext& access_context = replay_context.CurrentAccessContext();

        switch (entry.command_ref.type) {
            case CommandType::kBufferCopy: {
                replay_common(command_data.buffer_copy_commands[index], access_context);
                continue;
            }
            case CommandType::kBufferAccess: {
                replay_common(command_data.buffer_access_commands[index], access_context);
                continue;
            }
            case CommandType::kImageCopy: {
                replay_common(command_data.image_copy_commands[index], access_context);
                continue;
            }
            case CommandType::kBufferImageCopy: {
                replay_common(command_data.buffer_image_copy_commands[index], access_context);
                continue;
            }
            case CommandType::kImageBlit: {
                replay_common(command_data.image_blit_commands[index], access_context);
                continue;
            }
            case CommandType::kImageResolve: {
                replay_common(command_data.image_resolve_commands[index], access_context);
                continue;
            }
            case CommandType::kImageClear: {
                replay_common(command_data.image_clear_commands[index], access_context);
                continue;
            }
            case CommandType::kPipelineBarrier: {
                replay_common(command_data.barrier_commands[index], access_context);
                continue;
            }
            case CommandType::kSetEvent: {
                replay_common(command_data.set_event_commands[index], access_context);
                continue;
            }
            case CommandType::kResetEvent: {
                replay_common(command_data.reset_event_commands[index], access_context);
                continue;
            }
            case CommandType::kWaitEvents: {
                replay_common(command_data.wait_events_commands[index], access_context);
                continue;
            }
            case CommandType::kBeginRendering: {
                auto command = command_data.begin_rendering_commands[index].MakeCommand(command_data);
                command.render_pass_instance_id += replay_context.render_pass_instance_offset;
                command.rendering_instance.InitViewGens(replay_context.rendering_view_gens);
                replay_context.rendering_instance = command.rendering_instance;
                replay_context.rendering_instance_id = command.render_pass_instance_id;
                skip |= command.Validate(env, access_context, reporter);
                command.Apply(env, base_tag + reporter.replay_tag, access_context);
                continue;
            }
            case CommandType::kEndRendering: {
                if (!replay_context.rendering_instance) {
                    continue;
                }
                const EndRenderingCommand command{*replay_context.rendering_instance, replay_context.rendering_instance_id};
                skip |= command.Validate(env, access_context, reporter);
                command.Apply(env, base_tag + reporter.replay_tag, access_context);
                replay_context.rendering_instance.reset();
                replay_context.rendering_view_gens.clear();
                continue;
            }
            case CommandType::kBeginRenderPass: {
                const auto command = command_data.begin_render_pass_commands[index].MakeCommand(command_data);
                skip |= command.Validate(env, access_context, reporter);
                replay_context.BeginRenderPass(command);
                const ResourceUsageTag tag = base_tag + reporter.replay_tag;
                command.Apply(env, tag, *replay_context.render_pass_context);
                continue;
            }
            case CommandType::kNextSubpass: {
                const NextSubpassCommand command{};
                skip |= command.Validate(env, *replay_context.render_pass_context, reporter);
                replay_context.NextSubpass();
                const ResourceUsageTag tag = base_tag + reporter.replay_tag;
                command.Apply(env, tag, *replay_context.render_pass_context);
                continue;
            }
            case CommandType::kEndRenderPass: {
                const EndRenderPassCommand command{};
                skip |= command.Validate(env, *replay_context.render_pass_context, reporter);
                const ResourceUsageTag tag = base_tag + reporter.replay_tag;
                command.Apply(env, tag, *replay_context.render_pass_context, destination_access_context);
                replay_context.EndRenderPass();
                continue;
            }
            case CommandType::kShaderAccess: {
                replay_common(command_data.shader_access_commands[index], access_context);
                continue;
            }
            case CommandType::kDispatchIndirect: {
                replay_common(command_data.dispatch_indirect_commands[index], access_context);
                continue;
            }
            case CommandType::kTraceRays: {
                replay_common(command_data.trace_rays_commands[index], access_context);
                continue;
            }
            case CommandType::kDraw: {
                replay_draw(command_data.draw_commands[index], access_context);
                continue;
            }
            case CommandType::kDrawMulti: {
                replay_draw(command_data.draw_multi_commands[index], access_context);
                continue;
            }
            case CommandType::kDrawIndirect: {
                replay_draw(command_data.draw_indirect_commands[index], access_context);
                continue;
            }
            case CommandType::kDrawIndirectCount: {
                replay_draw(command_data.draw_indirect_count_commands[index], access_context);
                continue;
            }
            case CommandType::kDrawMeshTasks: {
                replay_draw(command_data.draw_mesh_tasks_commands[index], access_context);
                continue;
            }
            case CommandType::kBuildAccelerationStructures: {
                replay_common(command_data.build_acceleration_structures_commands[index], access_context);
                continue;
            }
            case CommandType::kAccelerationStructureCopy: {
                replay_common(command_data.acceleration_structure_copy_commands[index], access_context);
                continue;
            }
            case CommandType::kVideo: {
                replay_common(command_data.video_commands[index], access_context);
                continue;
            }
            case CommandType::kClearAttachments: {
                auto command = command_data.clear_attachments_commands[index].MakeCommand(command_data);
                command.render_pass_instance_id += replay_context.render_pass_instance_offset;
                skip |= command.Validate(env, access_context, reporter);
                command.Apply(env, base_tag + reporter.replay_tag, access_context);
                continue;
            }
            case CommandType::kQueryCopy: {
                replay_common(command_data.query_copy_commands[index], access_context);
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
    buffer_image_copy_commands.clear();
    image_blit_commands.clear();
    image_resolve_commands.clear();
    image_clear_commands.clear();
    barrier_commands.clear();
    set_event_commands.clear();
    reset_event_commands.clear();
    wait_events_commands.clear();
    begin_rendering_commands.clear();
    begin_render_pass_commands.clear();
    shader_access_commands.clear();
    dispatch_indirect_commands.clear();
    trace_rays_commands.clear();
    draw_commands.clear();
    draw_multi_commands.clear();
    draw_indirect_commands.clear();
    draw_indirect_count_commands.clear();
    draw_mesh_tasks_commands.clear();
    build_acceleration_structures_commands.clear();
    acceleration_structure_copy_commands.clear();
    video_commands.clear();
    clear_attachments_commands.clear();
    query_copy_commands.clear();

    buffers.clear();
    buffer_lookup.clear();
    last_buffer = nullptr;
    last_buffer_index = 0;
    images.clear();
    image_lookup.clear();
    last_image = nullptr;
    last_image_index = 0;
    image_views.clear();
    image_view_lookup.clear();
    last_image_view = nullptr;
    render_passes.clear();
    pipelines.clear();
    pipeline_lookup.clear();
    last_pipeline = nullptr;
    buffer_copy_regions.clear();
    image_copy_regions.clear();
    buffer_image_copy_regions.clear();
    image_blit_regions.clear();
    image_resolve_regions.clear();
    image_clear_ranges.clear();
    barrier_sets.clear();
    events.clear();
    rendering_attachments.clear();
    descriptor_buffer_accesses.clear();
    descriptor_image_accesses.clear();
    trace_rays_buffer_accesses.clear();
    vertex_input_accesses.clear();
    multi_draw_vertex_bindings.clear();
    multi_draw_ranges.clear();
    acceleration_structure_build_accesses.clear();
    video_picture_accesses.clear();
    clear_attachments.clear();
    clear_rects.clear();

    descriptor_sets.clear();
    descriptor_set_lookup.clear();
}

uint32_t CommandData::AddBuffer(const vvl::Buffer& buffer) {
    if (last_buffer == &buffer) {
        return last_buffer_index;
    }
    const auto [it, inserted] = buffer_lookup.try_emplace(&buffer, uint32_t(buffers.size()));
    if (inserted) {
        buffers.emplace_back(std::static_pointer_cast<const vvl::Buffer>(buffer.shared_from_this()));
    }
    last_buffer = &buffer;
    last_buffer_index = it->second;
    return last_buffer_index;
}

uint32_t CommandData::AddImage(const vvl::Image& image) {
    if (last_image == &image) {
        return last_image_index;
    }
    const auto [it, inserted] = image_lookup.try_emplace(&image, uint32_t(images.size()));
    if (inserted) {
        images.emplace_back(std::static_pointer_cast<const vvl::Image>(image.shared_from_this()));
    }
    last_image = &image;
    last_image_index = it->second;
    return last_image_index;
}

uint32_t CommandData::AddRenderPass(const vvl::RenderPass& render_pass) {
    const uint32_t index = uint32_t(render_passes.size());
    render_passes.emplace_back(std::static_pointer_cast<const vvl::RenderPass>(render_pass.shared_from_this()));
    return index;
}

void CommandData::AddImageView(const vvl::ImageView& image_view) {
    if (last_image_view == &image_view) {
        return;
    }
    const auto [it, inserted] = image_view_lookup.emplace(&image_view);
    if (inserted) {
        image_views.emplace_back(std::static_pointer_cast<const vvl::ImageView>(image_view.shared_from_this()));
    }
    last_image_view = &image_view;
}

void CommandData::AddPipeline(const vvl::Pipeline& pipeline) {
    if (last_pipeline == &pipeline) {
        return;
    }
    const auto [it, inserted] = pipeline_lookup.emplace(&pipeline);
    if (inserted) {
        pipelines.emplace_back(std::static_pointer_cast<const vvl::Pipeline>(pipeline.shared_from_this()));
    }
    last_pipeline = &pipeline;
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

// Record-time validation of a command
template <typename Command, typename Context>
static bool ValidateRecording(const Command& command, const Context& access_context, const CommandBufferContext& cb_context,
                              const Location& loc) {
    const SyncEnvironment& env = cb_context.GetSyncEnvironment();
    std::vector<ReportedHazard> new_hazards;

    const bool skip = command.Validate(env, access_context, ErrorReporter{cb_context, loc, &new_hazards});

    // A skipped command is not recorded, its tag goes to the next command
    if (!skip) {
        cb_context.RecordReportedHazards(new_hazards);
    }
    return skip;
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
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool BufferCopyCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                 const ErrorReporter& reporter) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;

    for (const auto [region_index, region] : vvl::enumerate(regions)) {
        const AccessRange src_range = MakeRange(src_buffer, region.src_offset, region.size);
        auto src_hazard = access_context.DetectHazard(src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
        if (src_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, reporter, src_buffer.Handle());
            const std::string resource_description = validator.FormatHandle(src_buffer);
            const std::string error = validator.error_messages_.BufferCopyError(env, src_hazard, reporter, resource_description,
                                                                                uint32_t(region_index), src_range);
            skip |= reporter.ReportHazard(src_hazard, src_buffer.Handle(), objlist, reporter.loc, error);
        }
        const AccessRange dst_range = MakeRange(dst_buffer, region.dst_offset, region.size);
        auto dst_hazard = access_context.DetectHazard(dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
        if (dst_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, reporter, dst_buffer.Handle());
            const std::string resource_description = validator.FormatHandle(dst_buffer);
            const std::string error = validator.error_messages_.BufferCopyError(env, dst_hazard, reporter, resource_description,
                                                                                uint32_t(region_index), dst_range);
            skip |= reporter.ReportHazard(dst_hazard, dst_buffer.Handle(), objlist, reporter.loc, error);
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
    return ValidateRecording(*this, access_context, cb_context, loc);
}

bool BufferAccessCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                   const ErrorReporter& reporter) const {
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
    if (!reporter.IsReplay() && is_marker) {
        objlist.add(buffer.Handle());
    } else {
        objlist = BaseObjectList(env, reporter, buffer.Handle());
    }
    const char* buffer_name_prefix = GetBufferNamePrefix(buffer_name);
    const std::string resource_description = buffer_name_prefix + validator.FormatHandle(buffer.Handle());
    const std::string error = validator.error_messages_.BufferError(env, hazard, reporter, resource_description, range);
    return reporter.ReportHazard(hazard, buffer.Handle(), objlist, reporter.loc, error);
}

void BufferAccessCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    access_context.UpdateAccessState(buffer, access_index, range, ResourceUsageTagEx{tag, handle_index}, flags, env.queue_id);
}

StridedBufferAccessCommand StridedBufferAccessCommand::Storage::MakeCommand(const CommandData& command_data) const {
    return {*command_data.buffers[buffer_index], offset, count, stride, access_size, access_index, handle_index, buffer_name};
}

StridedBufferAccessCommand::Storage StridedBufferAccessCommand::MakeStorage(CommandData& command_data) const {
    assert(access_size <= std::numeric_limits<uint16_t>::max());
    return {offset, command_data.AddBuffer(buffer), count, stride, access_index, handle_index, uint16_t(access_size), buffer_name};
}

uint32_t StridedBufferAccessCommand::GetRangeCount() const {
    if (count == 0) {
        return 0;
    }
    // Merge into one range if adjacent accesses have no gaps
    return (stride == access_size) ? 1 : count;
}

AccessRange StridedBufferAccessCommand::GetRange(uint32_t index) const {
    // Merge into one range if adjacent accesses have no gaps, otherwise return the requested range
    const VkDeviceSize size = (stride == access_size) ? VkDeviceSize(count) * access_size : access_size;
    return MakeRange(offset + VkDeviceSize(index) * stride, size);
}

bool StridedBufferAccessCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                          const ErrorReporter& reporter) const {
    for (uint32_t i = 0; i < GetRangeCount(); i++) {
        const AccessRange range = GetRange(i);
        const HazardResult hazard = access_context.DetectHazard(buffer, access_index, range);
        if (hazard.IsHazard()) {
            const SyncValidator& validator = env.validator;
            const LogObjectList objlist = BaseObjectList(env, reporter, buffer.Handle());
            const std::string resource_description = GetBufferNamePrefix(buffer_name) + validator.FormatHandle(buffer.Handle());
            const std::string error = validator.error_messages_.BufferError(env, hazard, reporter, resource_description, range);
            return reporter.ReportHazard(hazard, buffer.Handle(), objlist, reporter.loc, error);
        }
    }
    return false;
}

void StridedBufferAccessCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    for (uint32_t i = 0; i < GetRangeCount(); i++) {
        access_context.UpdateAccessState(buffer, access_index, GetRange(i), ResourceUsageTagEx{tag, handle_index}, 0, env.queue_id);
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
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool ImageCopyCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                const ErrorReporter& reporter) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;

    for (const auto [region_index, region] : vvl::enumerate(regions)) {
        auto src_hazard = access_context.DetectHazard(src_image, RangeFromLayers(region.srcSubresource), region.srcOffset,
                                                      region.extent, SYNC_COPY_TRANSFER_READ);
        if (src_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, reporter, src_image.Handle());
            const std::string resource_description = validator.FormatHandle(src_image);
            const std::string error = validator.error_messages_.ImageCopyResolveBlitError(
                env, src_hazard, reporter, resource_description, uint32_t(region_index), region.srcOffset, region.extent,
                region.srcSubresource);
            skip |= reporter.ReportHazard(src_hazard, src_image.Handle(), objlist, reporter.loc, error);
        }
        auto dst_hazard = access_context.DetectHazard(dst_image, RangeFromLayers(region.dstSubresource), region.dstOffset,
                                                      region.extent, SYNC_COPY_TRANSFER_WRITE);
        if (dst_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, reporter, dst_image.Handle());
            const std::string resource_description = validator.FormatHandle(dst_image);
            const std::string error = validator.error_messages_.ImageCopyResolveBlitError(
                env, dst_hazard, reporter, resource_description, uint32_t(region_index), region.dstOffset, region.extent,
                region.dstSubresource);
            skip |= reporter.ReportHazard(dst_hazard, dst_image.Handle(), objlist, reporter.loc, error);
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

static VkOffset3D GetBlitOffset(const VkOffset3D offsets[2]) {
    return {std::min(offsets[0].x, offsets[1].x), std::min(offsets[0].y, offsets[1].y), std::min(offsets[0].z, offsets[1].z)};
}

static VkExtent3D GetBlitExtent(const VkOffset3D offsets[2]) {
    return {static_cast<uint32_t>(std::abs(offsets[1].x - offsets[0].x)),
            static_cast<uint32_t>(std::abs(offsets[1].y - offsets[0].y)),
            static_cast<uint32_t>(std::abs(offsets[1].z - offsets[0].z))};
}

ImageBlitCommand ImageBlitCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const VkImageBlit> regions;
    if (region_count != 0) {
        regions = vvl::make_span(&command_data.image_blit_regions[first_region], region_count);
    }
    return {*src_image, *dst_image, regions, src_handle_index, dst_handle_index};
}

ImageBlitCommand::Storage ImageBlitCommand::MakeStorage(CommandData& command_data) const {
    command_data.AddImage(src_image);
    command_data.AddImage(dst_image);

    const uint32_t first_region = uint32_t(command_data.image_blit_regions.size());
    const uint32_t region_count = uint32_t(regions.size());
    vvl::Append(command_data.image_blit_regions, regions);

    return {&src_image, &dst_image, first_region, region_count, src_handle_index, dst_handle_index};
}

small_vector<VkImageBlit, 1> ImageBlitCommand::MakeRegions(vvl::span<const VkImageBlit2> regions) {
    small_vector<VkImageBlit, 1> result;
    result.reserve(uint32_t(regions.size()));
    for (const VkImageBlit2& region : regions) {
        result.emplace_back(VkImageBlit{region.srcSubresource,
                                        {region.srcOffsets[0], region.srcOffsets[1]},
                                        region.dstSubresource,
                                        {region.dstOffsets[0], region.dstOffsets[1]}});
    }
    return result;
}

bool ImageBlitCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool ImageBlitCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                const ErrorReporter& reporter) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;

    for (const auto [region_index, region] : vvl::enumerate(regions)) {
        const VkOffset3D src_offset = GetBlitOffset(region.srcOffsets);
        const VkExtent3D src_extent = GetBlitExtent(region.srcOffsets);
        const auto src_hazard = access_context.DetectHazard(src_image, RangeFromLayers(region.srcSubresource), src_offset,
                                                            src_extent, SYNC_BLIT_TRANSFER_READ);
        if (src_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, reporter, src_image.Handle());
            const std::string resource_description = validator.FormatHandle(src_image);
            const std::string error = validator.error_messages_.ImageCopyResolveBlitError(
                env, src_hazard, reporter, resource_description, uint32_t(region_index), src_offset, src_extent,
                region.srcSubresource);
            skip |= reporter.ReportHazard(src_hazard, src_image.Handle(), objlist, reporter.loc, error);
        }
        const VkOffset3D dst_offset = GetBlitOffset(region.dstOffsets);
        const VkExtent3D dst_extent = GetBlitExtent(region.dstOffsets);
        const auto dst_hazard = access_context.DetectHazard(dst_image, RangeFromLayers(region.dstSubresource), dst_offset,
                                                            dst_extent, SYNC_BLIT_TRANSFER_WRITE);
        if (dst_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, reporter, dst_image.Handle());
            const std::string resource_description = validator.FormatHandle(dst_image);
            const std::string error = validator.error_messages_.ImageCopyResolveBlitError(
                env, dst_hazard, reporter, resource_description, uint32_t(region_index), dst_offset, dst_extent,
                region.dstSubresource);
            skip |= reporter.ReportHazard(dst_hazard, dst_image.Handle(), objlist, reporter.loc, error);
        }
        if (skip) {
            break;
        }
    }
    return skip;
}

void ImageBlitCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    const ResourceUsageTagEx src_tag_ex{tag, src_handle_index};
    const ResourceUsageTagEx dst_tag_ex{tag, dst_handle_index};

    for (const VkImageBlit& region : regions) {
        UpdateImageAccessState(access_context, src_image, SYNC_BLIT_TRANSFER_READ, RangeFromLayers(region.srcSubresource),
                               GetBlitOffset(region.srcOffsets), GetBlitExtent(region.srcOffsets), src_tag_ex, env.queue_id);
        UpdateImageAccessState(access_context, dst_image, SYNC_BLIT_TRANSFER_WRITE, RangeFromLayers(region.dstSubresource),
                               GetBlitOffset(region.dstOffsets), GetBlitExtent(region.dstOffsets), dst_tag_ex, env.queue_id);
    }
}

ImageResolveCommand ImageResolveCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const VkImageResolve> regions;
    if (region_count != 0) {
        regions = vvl::make_span(&command_data.image_resolve_regions[first_region], region_count);
    }
    return {*src_image, *dst_image, regions, src_handle_index, dst_handle_index};
}

ImageResolveCommand::Storage ImageResolveCommand::MakeStorage(CommandData& command_data) const {
    command_data.AddImage(src_image);
    command_data.AddImage(dst_image);

    const uint32_t first_region = uint32_t(command_data.image_resolve_regions.size());
    const uint32_t region_count = uint32_t(regions.size());
    vvl::Append(command_data.image_resolve_regions, regions);

    return {&src_image, &dst_image, first_region, region_count, src_handle_index, dst_handle_index};
}

small_vector<VkImageResolve, 1> ImageResolveCommand::MakeRegions(vvl::span<const VkImageResolve2> regions) {
    small_vector<VkImageResolve, 1> result;
    result.reserve(uint32_t(regions.size()));
    for (const VkImageResolve2& region : regions) {
        result.emplace_back(
            VkImageResolve{region.srcSubresource, region.srcOffset, region.dstSubresource, region.dstOffset, region.extent});
    }
    return result;
}

bool ImageResolveCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool ImageResolveCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                   const ErrorReporter& reporter) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;

    for (const auto [region_index, region] : vvl::enumerate(regions)) {
        auto src_hazard = access_context.DetectHazard(src_image, RangeFromLayers(region.srcSubresource), region.srcOffset,
                                                      region.extent, SYNC_RESOLVE_TRANSFER_READ);
        if (src_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, reporter, src_image.Handle());
            const std::string resource_description = validator.FormatHandle(src_image);
            const std::string error = validator.error_messages_.ImageCopyResolveBlitError(
                env, src_hazard, reporter, resource_description, uint32_t(region_index), region.srcOffset, region.extent,
                region.srcSubresource);
            skip |= reporter.ReportHazard(src_hazard, src_image.Handle(), objlist, reporter.loc, error);
        }
        auto dst_hazard = access_context.DetectHazard(dst_image, RangeFromLayers(region.dstSubresource), region.dstOffset,
                                                      region.extent, SYNC_RESOLVE_TRANSFER_WRITE);
        if (dst_hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, reporter, dst_image.Handle());
            const std::string resource_description = validator.FormatHandle(dst_image);
            const std::string error = validator.error_messages_.ImageCopyResolveBlitError(
                env, dst_hazard, reporter, resource_description, uint32_t(region_index), region.dstOffset, region.extent,
                region.dstSubresource);
            skip |= reporter.ReportHazard(dst_hazard, dst_image.Handle(), objlist, reporter.loc, error);
        }
        if (skip) {
            break;
        }
    }
    return skip;
}

void ImageResolveCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    const ResourceUsageTagEx src_tag_ex{tag, src_handle_index};
    const ResourceUsageTagEx dst_tag_ex{tag, dst_handle_index};

    for (const VkImageResolve& region : regions) {
        UpdateImageAccessState(access_context, src_image, SYNC_RESOLVE_TRANSFER_READ, RangeFromLayers(region.srcSubresource),
                               region.srcOffset, region.extent, src_tag_ex, env.queue_id);
        UpdateImageAccessState(access_context, dst_image, SYNC_RESOLVE_TRANSFER_WRITE, RangeFromLayers(region.dstSubresource),
                               region.dstOffset, region.extent, dst_tag_ex, env.queue_id);
    }
}

ImageClearCommand ImageClearCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const VkImageSubresourceRange> ranges;
    if (range_count != 0) {
        ranges = vvl::make_span(&command_data.image_clear_ranges[first_range], range_count);
    }
    return {*image, ranges, handle_index};
}

ImageClearCommand::Storage ImageClearCommand::MakeStorage(CommandData& command_data) const {
    command_data.AddImage(image);
    const uint32_t first_range = uint32_t(command_data.image_clear_ranges.size());
    const uint32_t range_count = uint32_t(ranges.size());
    vvl::Append(command_data.image_clear_ranges, ranges);
    return {&image, first_range, range_count, handle_index};
}

bool ImageClearCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool ImageClearCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                 const ErrorReporter& reporter) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;
    for (const auto [range_index, range] : vvl::enumerate(ranges)) {
        const auto hazard = access_context.DetectHazard(image, range, SYNC_CLEAR_TRANSFER_WRITE);
        if (hazard.IsHazard()) {
            const LogObjectList objlist = BaseObjectList(env, reporter, image.Handle());
            const std::string resource_description = validator.FormatHandle(image);
            const std::string error = validator.error_messages_.ImageClearError(env, hazard, reporter, resource_description,
                                                                                uint32_t(range_index), range);
            skip |= reporter.ReportHazard(hazard, image.Handle(), objlist, reporter.loc, error);
        }
    }
    return skip;
}

void ImageClearCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    const ResourceUsageTagEx tag_ex{tag, handle_index};
    const auto& image_state = SubState(image);
    for (const VkImageSubresourceRange& range : ranges) {
        ImageRangeGen range_gen = image_state.MakeImageRangeGen(range, false);
        access_context.UpdateAccessState(range_gen, SYNC_CLEAR_TRANSFER_WRITE, tag_ex, 0, env.queue_id);
    }
}

BufferImageCopyCommand BufferImageCopyCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const VkBufferImageCopy> regions;
    if (region_count != 0) {
        regions = {&command_data.buffer_image_copy_regions[first_region], region_count};
    }
    const bool buffer_to_image = (direction == Direction::kBufferToImage);
    const uint32_t buffer_handle_index = buffer_to_image ? first_handle_index : first_handle_index + 1;
    const uint32_t image_handle_index = buffer_to_image ? first_handle_index + 1 : first_handle_index;
    return {*buffer, *image, regions, direction, buffer_handle_index, image_handle_index};
}

BufferImageCopyCommand::Storage BufferImageCopyCommand::MakeStorage(CommandData& command_data) const {
    const uint32_t first_region = uint32_t(command_data.buffer_image_copy_regions.size());
    const bool buffer_to_image = (direction == Direction::kBufferToImage);

    const uint32_t src_handle_index = buffer_to_image ? buffer_handle_index : image_handle_index;
    [[maybe_unused]] const uint32_t dst_handle_index = buffer_to_image ? image_handle_index : buffer_handle_index;
    assert(dst_handle_index == src_handle_index + 1);  // storage keeps only the source handle index

    command_data.AddBuffer(buffer);
    command_data.AddImage(image);
    vvl::Append(command_data.buffer_image_copy_regions, regions);
    return {&buffer, &image, first_region, uint32_t(regions.size()), direction, src_handle_index};
}

small_vector<VkBufferImageCopy, 1> BufferImageCopyCommand::MakeRegions(vvl::span<const VkBufferImageCopy2> regions) {
    small_vector<VkBufferImageCopy, 1> result;
    result.reserve(uint32_t(regions.size()));
    for (const VkBufferImageCopy2& region : regions) {
        result.emplace_back(VkBufferImageCopy{region.bufferOffset, region.bufferRowLength, region.bufferImageHeight,
                                              region.imageSubresource, region.imageOffset, region.imageExtent});
    }
    return result;
}

bool BufferImageCopyCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool BufferImageCopyCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                      const ErrorReporter& reporter) const {
    const SyncValidator& validator = env.validator;

    const auto validate_buffer = [&](uint32_t region_index, const VkBufferImageCopy& region, SyncAccessIndex buffer_access) {
        const AccessRange range = MakeRange(region.bufferOffset, image.GetBufferSizeFromCopyImage(region));
        const auto hazard = access_context.DetectHazard(buffer, buffer_access, range);
        if (!hazard.IsHazard()) {
            return false;
        }
        const LogObjectList objlist = BaseObjectList(env, reporter, buffer.Handle());
        const std::string error =
            validator.error_messages_.BufferCopyError(env, hazard, reporter, validator.FormatHandle(buffer), region_index, range);
        return reporter.ReportHazard(hazard, buffer.Handle(), objlist, reporter.loc, error);
    };

    const auto validate_image = [&](uint32_t region_index, const VkBufferImageCopy& region, SyncAccessIndex image_access) {
        const VkImageSubresourceRange range = RangeFromLayers(region.imageSubresource);
        const auto hazard = access_context.DetectHazard(image, range, region.imageOffset, region.imageExtent, image_access);
        if (!hazard.IsHazard()) {
            return false;
        }
        const LogObjectList objlist = BaseObjectList(env, reporter, image.Handle());
        const std::string error =
            validator.error_messages_.ImageCopyResolveBlitError(env, hazard, reporter, validator.FormatHandle(image), region_index,
                                                                region.imageOffset, region.imageExtent, region.imageSubresource);
        return reporter.ReportHazard(hazard, image.Handle(), objlist, reporter.loc, error);
    };

    bool skip = false;
    for (const auto [index, region] : vvl::enumerate(regions)) {
        const uint32_t region_index = uint32_t(index);
        if (direction == Direction::kBufferToImage) {
            skip |= validate_buffer(region_index, region, SYNC_COPY_TRANSFER_READ);
            skip |= validate_image(region_index, region, SYNC_COPY_TRANSFER_WRITE);
        } else {
            skip |= validate_image(region_index, region, SYNC_COPY_TRANSFER_READ);
            skip |= validate_buffer(region_index, region, SYNC_COPY_TRANSFER_WRITE);
        }
        if (skip) {
            break;
        }
    }
    return skip;
}

void BufferImageCopyCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    const ResourceUsageTagEx buffer_tag_ex{tag, buffer_handle_index};
    const ResourceUsageTagEx image_tag_ex{tag, image_handle_index};

    for (const VkBufferImageCopy& region : regions) {
        const AccessRange range = MakeRange(region.bufferOffset, image.GetBufferSizeFromCopyImage(region));
        if (direction == Direction::kBufferToImage) {
            access_context.UpdateAccessState(buffer, SYNC_COPY_TRANSFER_READ, range, buffer_tag_ex, 0, env.queue_id);
            UpdateImageAccessState(access_context, image, SYNC_COPY_TRANSFER_WRITE, RangeFromLayers(region.imageSubresource),
                                   region.imageOffset, region.imageExtent, image_tag_ex, env.queue_id);
        } else {
            UpdateImageAccessState(access_context, image, SYNC_COPY_TRANSFER_READ, RangeFromLayers(region.imageSubresource),
                                   region.imageOffset, region.imageExtent, image_tag_ex, env.queue_id);
            access_context.UpdateAccessState(buffer, SYNC_COPY_TRANSFER_WRITE, range, buffer_tag_ex, 0, env.queue_id);
        }
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
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool BarrierCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                              const ErrorReporter& reporter) const {
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
            const LogObjectList objlist = BaseObjectList(env, reporter, image_state.Handle());
            const std::string resource_description = validator.FormatHandle(image_state.Handle());
            const std::string error =
                validator.error_messages_.ImageBarrierError(env, hazard, reporter, resource_description, image_barrier);
            skip |= reporter.ReportHazard(hazard, image_state.Handle(), objlist, reporter.loc, error);
        }
    }
    return skip;
}

void BarrierCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    ApplyBarrier(env, access_context, barrier_set, tag);
}

SetEventCommand SetEventCommand::Storage::MakeCommand(const CommandData&) const { return {*event, src_exec_scope, command}; }

SetEventCommand::Storage SetEventCommand::MakeStorage(CommandData& command_data) const {
    command_data.events.emplace_back(std::static_pointer_cast<const vvl::Event>(event.shared_from_this()));
    return {&event, src_exec_scope, command};
}

bool SetEventCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool SetEventCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                               const ErrorReporter& reporter) const {
    const Location command_loc(command);
    const Location& error_loc = reporter.IsReplay() ? command_loc : reporter.loc;

    return ValidateCmdSetEvent(env, event, src_exec_scope, reporter, error_loc);
}

void SetEventCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    // Capture the state at this execution of SetEvent for later inspection at wait time.
    // TODO: Profile the full access-map copy for event-heavy workloads. During submit replay this copies
    // the queue access map; consider sharing immutable state or storing only the event source scope.
    auto src_access_context = std::make_shared<AccessContext>(env.validator);
    src_access_context->InitFrom(access_context);

    ApplyCmdSetEvent(env, event, src_exec_scope, src_access_context, tag, command);
}

ResetEventCommand ResetEventCommand::Storage::MakeCommand(const CommandData&) const { return {*event, exec_scope, command}; }

ResetEventCommand::Storage ResetEventCommand::MakeStorage(CommandData& command_data) const {
    command_data.events.emplace_back(std::static_pointer_cast<const vvl::Event>(event.shared_from_this()));
    return {&event, exec_scope, command};
}

bool ResetEventCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool ResetEventCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                 const ErrorReporter& reporter) const {
    const Location command_loc(command);
    const Location& error_loc = reporter.IsReplay() ? command_loc : reporter.loc;
    return ValidateCmdResetEvent(env, event, exec_scope, reporter, error_loc);
}

void ResetEventCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    ApplyCmdResetEvent(env, event, tag, command);
}

WaitEventsCommand WaitEventsCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const std::shared_ptr<const vvl::Event>> events;
    if (event_count != 0) {
        events = vvl::make_span(&command_data.events[first_event], event_count);
    }
    vvl::span<const BarrierSet> barrier_sets;
    if (barrier_set_count != 0) {
        barrier_sets = vvl::make_span(&command_data.barrier_sets[first_barrier_set], barrier_set_count);
    }
    return {events, barrier_sets, command};
}

WaitEventsCommand::Storage WaitEventsCommand::MakeStorage(CommandData& command_data) const {
    const uint32_t first_event = uint32_t(command_data.events.size());
    const uint32_t first_barrier_set = uint32_t(command_data.barrier_sets.size());
    vvl::Append(command_data.events, events);
    vvl::Append(command_data.barrier_sets, barrier_sets);
    return {first_event, uint32_t(events.size()), first_barrier_set, uint32_t(barrier_sets.size()), command};
}

bool WaitEventsCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool WaitEventsCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                 const ErrorReporter& reporter) const {
    const Location command_loc(command);
    const Location& error_loc = reporter.IsReplay() ? command_loc : reporter.loc;

    bool skip = false;
    skip = ValidateCmdWaitEvents(env, events, reporter, error_loc);
    skip |= DetectCmdWaitEventsImageBarrierHazard(env, access_context, events, barrier_sets, reporter);
    return skip;
}

void WaitEventsCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    ApplyCmdWaitEvents(env, access_context, events, barrier_sets, tag, command);
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
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool BeginRenderingCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                     const ErrorReporter& reporter) const {
    return rendering_instance.ValidateBeginRendering(env, access_context, reporter, render_pass_instance_id);
}

void BeginRenderingCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    rendering_instance.RecordBeginRendering(access_context, render_pass_instance_id, tag, env.queue_id);
}

bool EndRenderingCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool EndRenderingCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                   const ErrorReporter& reporter) const {
    return rendering_instance.ValidateEndRendering(env, access_context, reporter, render_pass_instance_id);
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
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool BeginRenderPassCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                      const ErrorReporter& reporter) const {
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
    const AttachmentViewGenVector view_gens =
        RenderPassAccessContext::CreateAttachmentViewGen(render_pass, render_area, attachment_views);

    skip |= RenderPassAccessContext::ValidateLayoutTransitions(env, temp_context, render_pass, render_pass_instance_id, 0,
                                                               view_mask, view_gens, reporter);
    if (!skip) {
        // Simulate initial layout transitions in the temporary context before validating load operations
        RenderPassAccessContext::RecordLayoutTransitions(render_pass, 0, view_gens, kInvalidTag, temp_context);

        skip |= RenderPassAccessContext::ValidateLoadOperation(env, temp_context, render_pass, render_pass_instance_id, 0,
                                                               view_mask, view_gens, reporter);
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
    return ValidateRecording(*this, *render_pass_context, cb_context, loc);
}
bool NextSubpassCommand::Validate(const SyncEnvironment& env, const RenderPassAccessContext& render_pass_context,
                                  const ErrorReporter& reporter) const {
    return render_pass_context.ValidateNextSubpass(env, reporter);
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
    return ValidateRecording(*this, *render_pass_context, cb_context, loc);
}

bool EndRenderPassCommand::Validate(const SyncEnvironment& env, const RenderPassAccessContext& render_pass_context,
                                    const ErrorReporter& reporter) const {
    return render_pass_context.ValidateEndRenderPass(env, reporter);
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
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool ShaderAccessCommand::ValidateBufferShaderAccess(const SyncEnvironment& env, const AccessContext& access_context,
                                                     const ErrorReporter& reporter, const BufferAccess& buffer_access) const {
    HazardResult hazard = access_context.DetectHazard(*buffer_access.buffer, buffer_access.access_index, buffer_access.range);
    if (!hazard.IsHazard()) {
        return false;
    }
    const SyncValidator& validator = env.validator;
    const DescriptorInfo& info = buffer_access.info;

    LogObjectList objlist = BaseObjectList(env, reporter, info.resource_handle);
    if (info.resource_handle.type == kVulkanObjectTypeAccelerationStructureKHR) {
        objlist.add(buffer_access.buffer->Handle());
    }
    objlist.add(pipeline->Handle());

    const std::string resource_description = validator.FormatHandle(info.resource_handle);

    std::string error;
    if (info.resource_handle.type == kVulkanObjectTypeAccelerationStructureKHR) {
        error = validator.error_messages_.AccelerationStructureDescriptorError(
            env, hazard, reporter, resource_description, *pipeline, info.set, *info.descriptor_set, info.descriptor_type,
            info.binding, info.array_element, info.stage);
    } else {
        error = validator.error_messages_.BufferDescriptorError(env, hazard, reporter, resource_description, *pipeline, info.set,
                                                                *info.descriptor_set, info.descriptor_type, info.binding,
                                                                info.array_element, info.stage);
    }
    return reporter.ReportHazard(hazard, buffer_access.info.resource_handle, objlist, reporter.loc, error);
}

bool ShaderAccessCommand::ValidateImageShaderAccess(const SyncEnvironment& env, const AccessContext& access_context,
                                                    const ErrorReporter& reporter, const ImageViewAccess& image_access) const {
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

    LogObjectList objlist = BaseObjectList(env, reporter, info.resource_handle);
    objlist.add(pipeline->Handle());

    std::string resource_description = validator.FormatHandle(info.resource_handle);
    if (reporter.IsReplay() && image_access.image_view->image_state) {
        resource_description += " (" + validator.FormatHandle(image_access.image_view->image_state->Handle()) + ")";
    }

    const std::string error = validator.error_messages_.ImageDescriptorError(
        env, hazard, reporter, resource_description, *pipeline, info.set, *info.descriptor_set, info.descriptor_type, info.binding,
        info.array_element, info.stage, image_access.image_layout);

    return reporter.ReportHazard(hazard, image_access.info.resource_handle, objlist, reporter.loc, error);
}

bool ShaderAccessCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                   const ErrorReporter& reporter) const {
    bool skip = false;
    for (const BufferAccess& access : buffer_accesses) {
        skip |= ValidateBufferShaderAccess(env, access_context, reporter, access);
    }
    for (const ImageViewAccess& access : image_accesses) {
        skip |= ValidateImageShaderAccess(env, access_context, reporter, access);
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

void DescriptorAccesses::RegisterResources(CommandBufferContext& cb_context, ResourceUsageTag tag) {
    for (auto& access : buffer_accesses) {
        access.handle_index = cb_context.AddCommandHandle(tag, access.info.resource_handle).handle_index;
    }
    for (auto& access : image_accesses) {
        access.handle_index = cb_context.AddCommandHandle(tag, access.image_view->image_state->Handle()).handle_index;
    }
}

DispatchIndirectCommand DispatchIndirectCommand::Storage::MakeCommand(const CommandData& command_data) const {
    return {shader_access_storage.MakeCommand(command_data), indirect_access_storage.MakeCommand(command_data)};
}

DispatchIndirectCommand::Storage DispatchIndirectCommand::MakeStorage(CommandData& command_data) const {
    return {shader_accesses.MakeStorage(command_data), indirect_access.MakeStorage(command_data)};
}

bool DispatchIndirectCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool DispatchIndirectCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                       const ErrorReporter& reporter) const {
    bool skip = false;
    skip |= shader_accesses.Validate(env, access_context, reporter);
    skip |= indirect_access.Validate(env, access_context, reporter);
    return skip;
}

void DispatchIndirectCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    shader_accesses.Apply(env, tag, access_context);
    indirect_access.Apply(env, tag, access_context);
}

TraceRaysCommand TraceRaysCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const BufferAccessCommand> buffer_accesses;
    if (buffer_access_count) {
        buffer_accesses = {&command_data.trace_rays_buffer_accesses[first_buffer_access], buffer_access_count};
    }
    return {shader_access_storage.MakeCommand(command_data), buffer_accesses};
}

TraceRaysCommand::Storage TraceRaysCommand::MakeStorage(CommandData& command_data) const {
    const uint32_t first_buffer_access = uint32_t(command_data.trace_rays_buffer_accesses.size());
    const uint32_t buffer_access_count = uint32_t(buffer_accesses.size());
    for (const BufferAccessCommand& access : buffer_accesses) {
        command_data.AddBuffer(access.buffer);
        command_data.trace_rays_buffer_accesses.emplace_back(access);
    }
    return {shader_accesses.MakeStorage(command_data), first_buffer_access, buffer_access_count};
}

bool TraceRaysCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool TraceRaysCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                const ErrorReporter& reporter) const {
    bool skip = shader_accesses.Validate(env, access_context, reporter);
    for (const BufferAccessCommand& access : buffer_accesses) {
        skip |= access.Validate(env, access_context, reporter);
    }
    return skip;
}

void TraceRaysCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    shader_accesses.Apply(env, tag, access_context);
    for (const BufferAccessCommand& access : buffer_accesses) {
        access.Apply(env, tag, access_context);
    }
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
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool DrawAttachmentCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                     const ErrorReporter& reporter) const {
    if (render_pass_context) {
        return render_pass_context->ValidateDrawSubpassAttachment(env, reporter, pipeline, depth_write, stencil_write);
    } else if (rendering_instance) {
        return rendering_instance->ValidateDrawAttachments(env, access_context, reporter, render_pass_instance_id, pipeline,
                                                           depth_write, stencil_write);
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

VertexInputCommand VertexInputCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const Access> accesses;
    if (access_count != 0) {
        accesses = vvl::make_span(&command_data.vertex_input_accesses[first_access], access_count);
    }
    return {pipeline, accesses, access_index};
}

VertexInputCommand::Storage VertexInputCommand::MakeStorage(CommandData& command_data) const {
    if (pipeline) {
        command_data.AddPipeline(*pipeline);
    }
    for (const Access& access : accesses) {
        command_data.AddBuffer(*access.buffer);
    }
    const uint32_t first_access = uint32_t(command_data.vertex_input_accesses.size());
    vvl::Append(command_data.vertex_input_accesses, accesses);
    return {pipeline, first_access, uint32_t(accesses.size()), access_index};
}

bool VertexInputCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool VertexInputCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                  const ErrorReporter& reporter) const {
    bool skip = false;
    for (const Access& access : accesses) {
        const HazardResult hazard = access_context.DetectHazard(*access.buffer, access_index, access.range);
        if (hazard.IsHazard()) {
            const SyncValidator& validator = env.validator;
            LogObjectList objlist = BaseObjectList(env, reporter, access.buffer->Handle());
            if (pipeline) {
                objlist.add(pipeline->Handle());
            }
            const char* buffer_name = access_index == SYNC_INDEX_INPUT_INDEX_READ ? "index " : "vertex ";
            const std::string resource_description = buffer_name + validator.FormatHandle(*access.buffer);
            const std::string error =
                validator.error_messages_.BufferError(env, hazard, reporter, resource_description, access.range);
            skip |= reporter.ReportHazard(hazard, access.buffer->Handle(), objlist, reporter.loc, error);
        }
    }
    return skip;
}

void VertexInputCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    for (const Access& access : accesses) {
        access_context.UpdateAccessState(*access.buffer, access_index, access.range, ResourceUsageTagEx{tag, access.handle_index},
                                         0, env.queue_id);
    }
}

MultiDrawVertexInputCommand MultiDrawVertexInputCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const Binding> bindings;
    if (binding_count != 0) {
        bindings = vvl::make_span(&command_data.multi_draw_vertex_bindings[first_binding], binding_count);
    }
    vvl::span<const DrawRange> draws;
    if (draw_count != 0) {
        draws = vvl::make_span(&command_data.multi_draw_ranges[first_draw], draw_count);
    }
    return {pipeline, bindings, draws, access_index};
}

MultiDrawVertexInputCommand::Storage MultiDrawVertexInputCommand::MakeStorage(CommandData& command_data) const {
    if (pipeline) {
        command_data.AddPipeline(*pipeline);
    }
    for (const Binding& binding : bindings) {
        command_data.AddBuffer(*binding.buffer);
    }
    const uint32_t first_binding = uint32_t(command_data.multi_draw_vertex_bindings.size());
    vvl::Append(command_data.multi_draw_vertex_bindings, bindings);
    const uint32_t first_draw = uint32_t(command_data.multi_draw_ranges.size());
    vvl::Append(command_data.multi_draw_ranges, draws);
    return {pipeline, first_binding, uint32_t(bindings.size()), first_draw, uint32_t(draws.size()), access_index};
}

AccessRange MultiDrawVertexInputCommand::GetRange(const Binding& binding, const DrawRange& draw) const {
    const VkDeviceSize offset = binding.offset + VkDeviceSize(draw.first) * binding.stride;
    const VkDeviceSize size = draw.count ? VkDeviceSize(draw.count - 1) * binding.stride + binding.access_size : 0;
    return MakeRange(offset, size);
}

bool MultiDrawVertexInputCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                           const ErrorReporter& reporter) const {
    bool skip = false;
    for (const DrawRange& draw : draws) {
        for (const Binding& binding : bindings) {
            const AccessRange range = GetRange(binding, draw);
            const HazardResult hazard = access_context.DetectHazard(*binding.buffer, access_index, range);
            if (hazard.IsHazard()) {
                const SyncValidator& validator = env.validator;
                LogObjectList objlist = BaseObjectList(env, reporter, binding.buffer->Handle());
                if (pipeline) {
                    objlist.add(pipeline->Handle());
                }
                const char* buffer_name = (access_index == SYNC_INDEX_INPUT_INDEX_READ) ? "index " : "vertex ";
                const std::string resource_description = buffer_name + validator.FormatHandle(*binding.buffer);
                const std::string error = validator.error_messages_.BufferError(env, hazard, reporter, resource_description, range);
                skip |= reporter.ReportHazard(hazard, binding.buffer->Handle(), objlist, reporter.loc, error);
            }
        }
    }
    return skip;
}

void MultiDrawVertexInputCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    for (const DrawRange& draw : draws) {
        for (const Binding& binding : bindings) {
            access_context.UpdateAccessState(*binding.buffer, access_index, GetRange(binding, draw),
                                             ResourceUsageTagEx{tag, binding.handle_index}, 0, env.queue_id);
        }
    }
}

void VertexInputAccesses::RegisterResources(CommandBufferContext& cb_context, ResourceUsageTag tag) {
    for (auto& access : accesses) {
        access.handle_index = cb_context.AddCommandHandle(tag, access.buffer->Handle()).handle_index;
    }
}

void MultiDrawVertexInputAccesses::RegisterResources(CommandBufferContext& cb_context, ResourceUsageTag tag) {
    for (auto& binding : bindings) {
        binding.handle_index = cb_context.AddCommandHandle(tag, binding.buffer->Handle()).handle_index;
    }
}

DrawCommand DrawCommand::Storage::MakeCommand(const CommandData& command_data, RenderPassAccessContext* render_pass_context,
                                              const RenderingInstance* rendering_instance) const {
    return {shader_access_storage.MakeCommand(command_data), vertex_access_storage.MakeCommand(command_data),
            attachment_access_storage.MakeCommand(render_pass_context, rendering_instance)};
}

DrawCommand::Storage DrawCommand::MakeStorage(CommandData& command_data) const {
    return {shader_accesses.MakeStorage(command_data), vertex_accesses.MakeStorage(command_data),
            attachment_accesses.MakeStorage(command_data)};
}

bool DrawCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool DrawCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context, const ErrorReporter& reporter) const {
    bool skip = false;
    skip |= shader_accesses.Validate(env, access_context, reporter);
    skip |= vertex_accesses.Validate(env, access_context, reporter);
    skip |= attachment_accesses.Validate(env, access_context, reporter);
    return skip;
}

void DrawCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    shader_accesses.Apply(env, tag, access_context);
    vertex_accesses.Apply(env, tag, access_context);
    attachment_accesses.Apply(env, tag, access_context);
}

DrawMultiCommand DrawMultiCommand::Storage::MakeCommand(const CommandData& command_data,
                                                        RenderPassAccessContext* render_pass_context,
                                                        const RenderingInstance* rendering_instance) const {
    return {shader_access_storage.MakeCommand(command_data),
            attachment_access_storage.MakeCommand(render_pass_context, rendering_instance),
            vertex_access_storage.MakeCommand(command_data)};
}

DrawMultiCommand::Storage DrawMultiCommand::MakeStorage(CommandData& command_data) const {
    return {shader_accesses.MakeStorage(command_data), attachment_accesses.MakeStorage(command_data),
            vertex_accesses.MakeStorage(command_data)};
}

bool DrawMultiCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool DrawMultiCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                const ErrorReporter& reporter) const {
    bool skip = false;
    skip |= shader_accesses.Validate(env, access_context, reporter);
    skip |= attachment_accesses.Validate(env, access_context, reporter);
    skip |= vertex_accesses.Validate(env, access_context, reporter);
    return skip;
}

void DrawMultiCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    shader_accesses.Apply(env, tag, access_context);
    attachment_accesses.Apply(env, tag, access_context);
    vertex_accesses.Apply(env, tag, access_context);
}

DrawIndirectCommand DrawIndirectCommand::Storage::MakeCommand(const CommandData& command_data,
                                                              RenderPassAccessContext* render_pass_context,
                                                              const RenderingInstance* rendering_instance) const {
    return {shader_access_storage.MakeCommand(command_data),
            attachment_access_storage.MakeCommand(render_pass_context, rendering_instance),
            indirect_access_storage.MakeCommand(command_data)};
}

DrawIndirectCommand::Storage DrawIndirectCommand::MakeStorage(CommandData& command_data) const {
    return {shader_accesses.MakeStorage(command_data), attachment_accesses.MakeStorage(command_data),
            indirect_access.MakeStorage(command_data)};
}

bool DrawIndirectCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool DrawIndirectCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                   const ErrorReporter& reporter) const {
    bool skip = false;
    skip |= shader_accesses.Validate(env, access_context, reporter);
    skip |= attachment_accesses.Validate(env, access_context, reporter);
    skip |= indirect_access.Validate(env, access_context, reporter);
    // TODO: shader instrumentation support is needed to read indirect buffer content (ValidateDrawVertex)
    return skip;
}

void DrawIndirectCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    shader_accesses.Apply(env, tag, access_context);
    attachment_accesses.Apply(env, tag, access_context);
    indirect_access.Apply(env, tag, access_context);
    // TODO: shader instrumentation support is needed to read indirect buffer content (RecordDrawVertexIndex)
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
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool DrawIndirectCountCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                        const ErrorReporter& reporter) const {
    bool skip = false;
    skip |= shader_accesses.Validate(env, access_context, reporter);
    skip |= attachment_accesses.Validate(env, access_context, reporter);
    skip |= count_access.Validate(env, access_context, reporter);
    // TODO: shader instrumentation support is needed to read indirect buffer content (ValidateDrawVertex)
    return skip;
}

void DrawIndirectCountCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    shader_accesses.Apply(env, tag, access_context);
    attachment_accesses.Apply(env, tag, access_context);
    count_access.Apply(env, tag, access_context);
    // TODO: shader instrumentation support is needed to read indirect buffer content (RecordDrawVertexIndex)
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
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool DrawMeshTasksCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                    const ErrorReporter& reporter) const {
    bool skip = false;
    skip |= shader_accesses.Validate(env, access_context, reporter);
    skip |= attachment_accesses.Validate(env, access_context, reporter);
    return skip;
}

void DrawMeshTasksCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    shader_accesses.Apply(env, tag, access_context);
    attachment_accesses.Apply(env, tag, access_context);
}

static SyncAccessIndex GetAccessIndex(BuildAccelerationStructuresCommand::AccessType type) {
    switch (type) {
        case BuildAccelerationStructuresCommand::AccessType::kScratch:
        case BuildAccelerationStructuresCommand::AccessType::kDestination:
            return SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_WRITE;
        case BuildAccelerationStructuresCommand::AccessType::kSource:
            return SYNC_ACCELERATION_STRUCTURE_BUILD_ACCELERATION_STRUCTURE_READ;
        default:
            return SYNC_ACCELERATION_STRUCTURE_BUILD_SHADER_READ;
    }
}

BuildAccelerationStructuresCommand BuildAccelerationStructuresCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const Access> accesses;
    if (access_count != 0) {
        accesses = vvl::make_span(&command_data.acceleration_structure_build_accesses[first_access], access_count);
    }
    return {accesses};
}

BuildAccelerationStructuresCommand::Storage BuildAccelerationStructuresCommand::MakeStorage(CommandData& command_data) const {
    const uint32_t first_access = uint32_t(command_data.acceleration_structure_build_accesses.size());
    vvl::Append(command_data.acceleration_structure_build_accesses, accesses);
    for (const Access& access : accesses) {
        command_data.AddBuffer(*access.buffer);
    }
    return {first_access, uint32_t(accesses.size())};
}

bool BuildAccelerationStructuresCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool BuildAccelerationStructuresCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                                  const ErrorReporter& reporter) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;
    for (const Access& access : accesses) {
        const auto hazard = access_context.DetectHazard(*access.buffer, GetAccessIndex(access.type), access.range);
        if (!hazard.IsHazard()) {
            continue;
        }
        LogObjectList objlist = BaseObjectList(env, reporter, access.buffer->Handle());
        std::string error;
        if (access.acceleration_structure != VK_NULL_HANDLE) {
            objlist.add(access.acceleration_structure);
            const Location info_loc(vvl::Func::vkCmdBuildAccelerationStructuresKHR, vvl::Field::pInfos, access.info_index);
            const auto field =
                access.type == AccessType::kSource ? vvl::Field::srcAccelerationStructure : vvl::Field::dstAccelerationStructure;
            error = validator.error_messages_.AccelerationStructureError(
                env, hazard, reporter, validator.FormatHandle(access.buffer->Handle()), access.range, access.acceleration_structure,
                info_loc.dot(field));
        } else {
            const char* description = nullptr;
            switch (access.type) {
                case AccessType::kScratch:
                    description = "scratch buffer ";
                    break;
                case AccessType::kVertex:
                    description = "vertex data ";
                    break;
                case AccessType::kIndex:
                    description = "index data ";
                    break;
                case AccessType::kTransform:
                    description = "transform data ";
                    break;
                case AccessType::kAABB:
                    description = "aabb data ";
                    break;
                case AccessType::kInstance:
                    description = "instance data ";
                    break;
                default:
                    assert(false);
                    continue;
            }
            const std::string resource_description = description + validator.FormatHandle(access.buffer->Handle());
            error = validator.error_messages_.BufferError(env, hazard, reporter, resource_description, access.range);
        }
        skip |= reporter.ReportHazard(hazard, access.buffer->Handle(), objlist, reporter.loc, error);
    }
    return skip;
}

void BuildAccelerationStructuresCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    for (const Access& access : accesses) {
        access_context.UpdateAccessState(*access.buffer, GetAccessIndex(access.type), access.range,
                                         ResourceUsageTagEx{tag, access.handle_index}, 0, env.queue_id);
    }
}

AccelerationStructureCopyCommand AccelerationStructureCopyCommand::Storage::MakeCommand(const CommandData&) const {
    return {src, dst};
}

AccelerationStructureCopyCommand::Storage AccelerationStructureCopyCommand::MakeStorage(CommandData& command_data) const {
    if (src.buffer) {
        command_data.AddBuffer(*src.buffer);
    }
    if (dst.buffer) {
        command_data.AddBuffer(*dst.buffer);
    }
    return {src, dst};
}

bool AccelerationStructureCopyCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool AccelerationStructureCopyCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                                const ErrorReporter& reporter) const {
    const SyncValidator& validator = env.validator;
    const Location info_loc(vvl::Func::Empty, vvl::Field::pInfo);

    const auto validate_access = [&](const Access& access, SyncAccessIndex access_index, vvl::Field field) {
        if (!access.buffer) {
            return false;
        }
        const auto hazard = access_context.DetectHazard(*access.buffer, access_index, access.range);
        if (!hazard.IsHazard()) {
            return false;
        }
        LogObjectList objlist = BaseObjectList(env, reporter, access.buffer->Handle());
        objlist.add(access.acceleration_structure);
        const std::string error = validator.error_messages_.AccelerationStructureError(
            env, hazard, reporter, validator.FormatHandle(access.buffer->Handle()), access.range, access.acceleration_structure,
            info_loc.dot(field));
        return reporter.ReportHazard(hazard, access.buffer->Handle(), objlist, reporter.loc, error);
    };
    bool skip = false;
    skip |= validate_access(src, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_READ, vvl::Field::src);
    skip |= validate_access(dst, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_WRITE, vvl::Field::dst);
    return skip;
}

void AccelerationStructureCopyCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    if (src.buffer) {
        access_context.UpdateAccessState(*src.buffer, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_READ, src.range,
                                         ResourceUsageTagEx{tag, src.handle_index}, 0, env.queue_id);
    }
    if (dst.buffer) {
        access_context.UpdateAccessState(*dst.buffer, SYNC_ACCELERATION_STRUCTURE_COPY_ACCELERATION_STRUCTURE_WRITE, dst.range,
                                         ResourceUsageTagEx{tag, dst.handle_index}, 0, env.queue_id);
    }
}

VideoCommand VideoCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const PictureAccess> pictures;
    if (picture_count != 0) {
        pictures = vvl::make_span(&command_data.video_picture_accesses[first_picture], picture_count);
    }
    return {operation, *bitstream_buffer, bitstream_range, pictures, bitstream_handle_index};
}

VideoCommand::Storage VideoCommand::MakeStorage(CommandData& command_data) const {
    command_data.AddBuffer(bitstream_buffer);
    const uint32_t first_picture = uint32_t(command_data.video_picture_accesses.size());
    vvl::Append(command_data.video_picture_accesses, pictures);
    for (const PictureAccess& picture : pictures) {
        command_data.AddImageView(*picture.view);
    }
    return {bitstream_range, &bitstream_buffer, first_picture, uint32_t(pictures.size()), bitstream_handle_index, operation};
}

static SyncAccessIndex GetVideoPictureAccessIndex(VideoCommand::Operation operation, VideoCommand::PictureType type) {
    const bool write = (type == VideoCommand::PictureType::kOutput) || (type == VideoCommand::PictureType::kReconstructed);
    if (operation == VideoCommand::Operation::kDecode) {
        return write ? SYNC_VIDEO_DECODE_VIDEO_DECODE_WRITE : SYNC_VIDEO_DECODE_VIDEO_DECODE_READ;
    } else {
        return write ? SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE : SYNC_VIDEO_ENCODE_VIDEO_ENCODE_READ;
    }
}

static ImageRangeGen MakeVideoPictureRangeGen(const VideoCommand::PictureAccess& picture) {
    const VkOffset3D offset = {picture.effective_offset.x, picture.effective_offset.y, 0};
    const VkExtent3D extent = {picture.effective_extent.width, picture.effective_extent.height, 1};
    if (picture.type == VideoCommand::PictureType::kQuantizationMap) {
        return MakeImageRangeGen(*picture.view, offset, extent);
    } else {
        const ImageSubState& image_substate = SubState(*picture.view->image_state);
        return image_substate.MakeImageRangeGen(picture.subresource_range, offset, extent, false);
    }
}

bool VideoCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool VideoCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context, const ErrorReporter& reporter) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;

    const SyncAccessIndex bitstream_access_index =
        (operation == Operation::kDecode) ? SYNC_VIDEO_DECODE_VIDEO_DECODE_READ : SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE;

    const auto bitstream_hazard = access_context.DetectHazard(bitstream_buffer, bitstream_access_index, bitstream_range);
    if (bitstream_hazard.IsHazard()) {
        // TODO: Unify record-time video object lists with other commands after conversion
        const LogObjectList objlist = reporter.IsReplay() ? BaseObjectList(env, reporter, bitstream_buffer.Handle())
                                                          : LogObjectList(bitstream_buffer.Handle());
        const std::string resource_description = "bitstream buffer " + validator.FormatHandle(bitstream_buffer.Handle());
        const std::string error =
            validator.error_messages_.BufferError(env, bitstream_hazard, reporter, resource_description, bitstream_range);
        skip |= reporter.ReportHazard(bitstream_hazard, bitstream_buffer.Handle(), objlist, reporter.loc, error);
    }
    for (const PictureAccess& picture : pictures) {
        auto range_gen = MakeVideoPictureRangeGen(picture);
        const SyncAccessIndex picture_access_index = GetVideoPictureAccessIndex(operation, picture.type);
        const auto hazard = access_context.DetectHazard(range_gen, picture_access_index);
        if (!hazard.IsHazard()) {
            continue;
        }
        const Location info_loc(vvl::Func::Empty,
                                operation == Operation::kDecode ? vvl::Field::pDecodeInfo : vvl::Field::pEncodeInfo);
        std::ostringstream ss;
        switch (picture.type) {
            case PictureType::kOutput:
                ss << "decode output picture " << info_loc.dot(vvl::Field::dstPictureResource).Fields();
                break;
            case PictureType::kInput:
                ss << "encode input picture " << info_loc.dot(vvl::Field::srcPictureResource).Fields();
                break;
            case PictureType::kReconstructed:
                ss << "reconstructed picture "
                   << info_loc.dot(vvl::Field::pSetupReferenceSlot).dot(vvl::Field::pPictureResource).Fields();
                break;
            case PictureType::kReference:
                ss << "reference picture " << picture.reference_index << " "
                   << info_loc.dot(vvl::Field::pReferenceSlots, picture.reference_index).dot(vvl::Field::pPictureResource).Fields();
                break;
            case PictureType::kQuantizationMap:
                ss << "quantization map " << info_loc.dot(vvl::Field::quantizationMap).Fields();
                break;
        }
        ss << " ";
        if (picture.type == PictureType::kQuantizationMap) {
            VkVideoEncodeQuantizationMapInfoKHR map_info = vku::InitStructHelper();
            map_info.quantizationMap = picture.view->VkHandle();
            map_info.quantizationMapExtent = picture.coded_extent;
            FormatVideoQuantizationMap(validator, map_info, ss);
        } else {
            VkVideoPictureResourceInfoKHR picture_info = vku::InitStructHelper();
            picture_info.imageViewBinding = picture.view->VkHandle();
            picture_info.codedOffset = picture.coded_offset;
            picture_info.codedExtent = picture.coded_extent;
            picture_info.baseArrayLayer = picture.base_array_layer;
            FormatVideoPictureResouce(validator, picture_info, ss);
        }
        const LogObjectList objlist =
            reporter.IsReplay() ? BaseObjectList(env, reporter, picture.view->Handle()) : LogObjectList(picture.view->Handle());
        const std::string error = validator.error_messages_.VideoError(env, hazard, reporter, ss.str());
        skip |= reporter.ReportHazard(hazard, picture.view->Handle(), objlist, reporter.loc, error);
    }
    return skip;
}

void VideoCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    const SyncAccessIndex buffer_access_index =
        (operation == Operation::kDecode) ? SYNC_VIDEO_DECODE_VIDEO_DECODE_READ : SYNC_VIDEO_ENCODE_VIDEO_ENCODE_WRITE;
    access_context.UpdateAccessState(bitstream_buffer, buffer_access_index, bitstream_range,
                                     ResourceUsageTagEx{tag, bitstream_handle_index}, 0, env.queue_id);

    for (const PictureAccess& picture : pictures) {
        auto range_gen = MakeVideoPictureRangeGen(picture);
        const SyncAccessIndex picture_access_index = GetVideoPictureAccessIndex(operation, picture.type);
        access_context.UpdateAccessState(range_gen, picture_access_index, ResourceUsageTagEx{tag}, 0, env.queue_id);
    }
}

ClearAttachmentsCommand::Storage ClearAttachmentsCommand::MakeStorage(CommandData& command_data) const {
    const uint32_t first_attachment = uint32_t(command_data.clear_attachments.size());
    vvl::Append(command_data.clear_attachments, attachments);
    for (const Attachment& attachment : attachments) {
        command_data.AddImageView(*attachment.view);
    }
    const uint32_t first_rect = uint32_t(command_data.clear_rects.size());
    vvl::Append(command_data.clear_rects, rects);
    return {first_attachment, uint32_t(attachments.size()), first_rect, uint32_t(rects.size()),
            view_mask,        render_pass_instance_id,      subpass};
}

ClearAttachmentsCommand ClearAttachmentsCommand::Storage::MakeCommand(const CommandData& command_data) const {
    vvl::span<const Attachment> attachments;
    if (attachment_count) {
        attachments = {&command_data.clear_attachments[first_attachment], attachment_count};
    }
    vvl::span<const VkClearRect> rects;
    if (rect_count) {
        rects = {&command_data.clear_rects[first_rect], rect_count};
    }
    return {attachments, rects, view_mask, render_pass_instance_id, subpass};
}

static std::optional<VkImageSubresourceRange> RestrictSubresourceRangeToClearLayers(
    const VkImageSubresourceRange& normalized_subresource_range, uint32_t clear_first_layer, uint32_t clear_layer_count) {
    // Contract of this function
    assert(normalized_subresource_range.layerCount != VK_REMAINING_ARRAY_LAYERS);
    // According to spec
    assert(clear_layer_count != VK_REMAINING_ARRAY_LAYERS);

    const uint32_t first = std::max(normalized_subresource_range.baseArrayLayer, clear_first_layer);
    const uint32_t last_range = normalized_subresource_range.baseArrayLayer + normalized_subresource_range.layerCount;
    const uint32_t last_clear = clear_first_layer + clear_layer_count;
    const uint32_t last = std::min(last_range, last_clear);

    if (first >= last) {
        return {};
    }

    std::optional<VkImageSubresourceRange> result = normalized_subresource_range;
    result->baseArrayLayer = first;
    result->layerCount = last - first;
    return result;
}

bool ClearAttachmentsCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCurrentAccessContext(), cb_context, loc);
}

bool ClearAttachmentsCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                       const ErrorReporter& reporter) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;

    auto report_hazard = [&skip, &env, &reporter, this, &validator](const HazardResult& hazard, const Attachment& attachment,
                                                                    bool color, uint32_t rect_index, const VkClearRect& rect) {
        std::ostringstream ss;
        ss << string_VkImageAspectFlags(attachment.original_aspects);
        if (color) {
            ss << " aspect of color attachment " << attachment.color_attachment;
            ss << " (" << validator.FormatHandle(*attachment.view) << ")";
        } else {
            ss << " aspect(s) of depth-stencil attachment (" << validator.FormatHandle(*attachment.view) << ")";
        }
        if (subpass != vvl::kNoIndex32) {
            ss << " in subpass " << subpass;
        }
        const LogObjectList objlist = BaseObjectList(env, reporter, attachment.view->Handle());
        const std::string error = validator.error_messages_.ClearAttachmentError(env, hazard, reporter, ss.str(),
                                                                                 attachment.original_aspects, rect_index, rect);
        skip |= reporter.ReportHazard(hazard, attachment.view->Handle(), objlist, reporter.loc, error);
    };

    for (const Attachment& attachment : attachments) {
        const bool color = (attachment.effective_aspects & kColorAspects) != 0;
        const SyncOrdering ordering = color ? SyncOrdering::kColorAttachment : SyncOrdering::kDepthStencilAttachment;
        const AttachmentAccess attachment_access{AttachmentAccessType::Access, ordering, render_pass_instance_id, subpass};

        // Depth/stencil clears execute in both early and late stages. Track the most recent access (late)
        const SyncAccessIndex access_index =
            color ? SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE : SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE;

        for (const auto [rect_index, rect] : vvl::enumerate(rects)) {
            auto subresource_range = RestrictSubresourceRangeToClearLayers(attachment.view->normalized_subresource_range,
                                                                           rect.baseArrayLayer, rect.layerCount);
            if (!subresource_range) {
                continue;
            }
            subresource_range->aspectMask = attachment.effective_aspects;

            if (view_mask == 0) {
                const auto hazard = access_context.DetectAttachmentHazard(*attachment.view->image_state, *subresource_range,
                                                                          attachment.view->is_depth_sliced, access_index,
                                                                          attachment_access, env.queue_id);
                if (hazard.IsHazard()) {
                    report_hazard(hazard, attachment, color, uint32_t(rect_index), rect);
                }
            } else {
                const ImageSubState& sub_state = SubState(*attachment.view->image_state);
                const VkImageSubresourceRange& attachment_subresource = attachment.view->normalized_subresource_range;
                const auto view_indices = GetSetBitIndices(view_mask);

                for (uint32_t view_index : view_indices) {
                    if (view_index < attachment_subresource.layerCount) {
                        VkImageSubresourceRange view_subresource = attachment_subresource;
                        view_subresource.baseArrayLayer += view_index;
                        view_subresource.layerCount = 1;

                        auto range_gen = sub_state.MakeImageRangeGen(view_subresource, attachment.view->is_depth_sliced);
                        const auto hazard =
                            access_context.DetectAttachmentHazard(range_gen, access_index, attachment_access, env.queue_id);
                        if (hazard.IsHazard()) {
                            report_hazard(hazard, attachment, color, uint32_t(rect_index), rect);
                        }
                    }
                }
            }
        }
    }
    return skip;
}

void ClearAttachmentsCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    for (const Attachment& attachment : attachments) {
        const ImageSubState& sub_state = SubState(*attachment.view->image_state);
        const bool color = (attachment.effective_aspects & kColorAspects) != 0;
        const SyncOrdering ordering = color ? SyncOrdering::kColorAttachment : SyncOrdering::kDepthStencilAttachment;
        const AttachmentAccess attachment_access{AttachmentAccessType::Access, ordering, render_pass_instance_id, subpass};

        const SyncAccessIndex access_index =
            color ? SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE : SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE;

        for (const VkClearRect& rect : rects) {
            auto subresource_range = RestrictSubresourceRangeToClearLayers(attachment.view->normalized_subresource_range,
                                                                           rect.baseArrayLayer, rect.layerCount);
            if (!subresource_range) {
                continue;
            }
            subresource_range->aspectMask = attachment.effective_aspects;
            // NOTE: when we teach ImageRangeGen to work with view masks all logic will be much simplified
            if (view_mask == 0) {
                auto range_gen = sub_state.MakeImageRangeGen(*subresource_range, attachment.view->is_depth_sliced);
                access_context.UpdateAttachmentAccessState(range_gen, access_index, attachment_access, ResourceUsageTagEx{tag},
                                                           env.queue_id);
            } else {
                const VkImageSubresourceRange& attachment_subresource = attachment.view->normalized_subresource_range;
                const auto view_indices = GetSetBitIndices(view_mask);

                for (uint32_t view_index : view_indices) {
                    if (view_index < attachment_subresource.layerCount) {
                        VkImageSubresourceRange view_subresource = attachment_subresource;
                        view_subresource.baseArrayLayer += view_index;
                        view_subresource.layerCount = 1;
                        auto range_gen = sub_state.MakeImageRangeGen(view_subresource, attachment.view->is_depth_sliced);
                        access_context.UpdateAttachmentAccessState(range_gen, access_index, attachment_access,
                                                                   ResourceUsageTagEx{tag}, env.queue_id);
                    }
                }
            }
        }
    }
}

QueryCopyCommand QueryCopyCommand::Storage::MakeCommand(const CommandData& command_data) const {
    return {*dst_buffer, range, query_pool, handle_index};
}

QueryCopyCommand::Storage QueryCopyCommand::MakeStorage(CommandData& command_data) const {
    command_data.AddBuffer(dst_buffer);
    return {&dst_buffer, range, query_pool, handle_index};
}

bool QueryCopyCommand::Validate(const CommandBufferContext& cb_context, const Location& loc) const {
    return ValidateRecording(*this, cb_context.GetCbAccessContext(), cb_context, loc);
}

bool QueryCopyCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context,
                                const ErrorReporter& reporter) const {
    const auto hazard = access_context.DetectHazard(dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
    if (!hazard.IsHazard()) {
        return false;
    }
    const SyncValidator& validator = env.validator;
    LogObjectList objlist = BaseObjectList(env, reporter, VulkanTypedHandle(query_pool, kVulkanObjectTypeQueryPool));
    objlist.add(dst_buffer.Handle());
    const std::string resource_description = "dstBuffer " + validator.FormatHandle(dst_buffer.Handle());
    const std::string error = validator.error_messages_.BufferError(env, hazard, reporter, resource_description, range);
    return reporter.ReportHazard(hazard, dst_buffer.Handle(), objlist, reporter.loc, error);
}

void QueryCopyCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    access_context.UpdateAccessState(dst_buffer, SYNC_COPY_TRANSFER_WRITE, range, ResourceUsageTagEx{tag, handle_index}, 0,
                                     env.queue_id);
}

}  // namespace syncval
