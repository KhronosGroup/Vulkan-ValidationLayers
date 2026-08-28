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
#include "error_message/logging.h"
#include "utils/image_utils.h"

namespace syncval {

bool ReplayCommands(SyncEnvironment& env, AccessContext& access_context, const CommandBufferContext& cb_context,
                    ResourceUsageTag base_tag, const Location& loc) {
    bool skip = false;
    const CommandData& command_data = cb_context.GetCommandData();

    for (const CommandEntry& entry : cb_context.GetCommands()) {
        const ResourceUsageTag tag = base_tag + entry.tag;
        std::visit(
            [&](const auto& storage) {
                const auto& command = storage.MakeCommand(command_data);
                const bool command_skip = command.Validate(env, access_context, cb_context, entry.tag, loc);
                if (!command_skip) {
                    command.Apply(env, tag, access_context);
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
                                 const CommandBufferContext& cb_context, ResourceUsageTag command_tag, const Location& loc) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;
    const bool submit_time = env.handle.type == kVulkanObjectTypeQueue;

    // TODO: Remove SubmitTimeError and extend BufferCopyError with submit-time details after
    // command-base validation replaces current model. Until then, try to preserve identical
    // error output so old and new can be compared during development.

    for (const auto [region_index, region] : vvl::enumerate(regions)) {
        const AccessRange src_range = MakeRange(src_buffer, region.src_offset, region.size);
        auto src_hazard = access_context.DetectHazard(src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
        if (src_hazard.IsHazard()) {
            const LogObjectList objlist = submit_time ? LogObjectList(env.handle, cb_context.GetCBState().Handle())
                                                      : LogObjectList(cb_context.GetCBState().Handle(), src_buffer.Handle());
            const std::string error =
                submit_time
                    ? validator.error_messages_.SubmitTimeError(env, src_hazard, cb_context, command_tag, loc.index,
                                                                validator.FormatHandle(src_buffer))
                    : validator.error_messages_.BufferCopyError(env, src_hazard, loc.function, validator.FormatHandle(src_buffer),
                                                                uint32_t(region_index), src_range);
            skip |= validator.SyncError(src_hazard.Hazard(), objlist, loc, error);
        }
        const AccessRange dst_range = MakeRange(dst_buffer, region.dst_offset, region.size);
        auto dst_hazard = access_context.DetectHazard(dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
        if (dst_hazard.IsHazard()) {
            const LogObjectList objlist = submit_time ? LogObjectList(env.handle, cb_context.GetCBState().Handle())
                                                      : LogObjectList(cb_context.GetCBState().Handle(), dst_buffer.Handle());
            const std::string error =
                submit_time
                    ? validator.error_messages_.SubmitTimeError(env, dst_hazard, cb_context, command_tag, loc.index,
                                                                validator.FormatHandle(dst_buffer))
                    : validator.error_messages_.BufferCopyError(env, dst_hazard, loc.function, validator.FormatHandle(dst_buffer),
                                                                uint32_t(region_index), dst_range);
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
    const bool submit_time = env.handle.type == kVulkanObjectTypeQueue;

    for (const auto [region_index, region] : vvl::enumerate(regions)) {
        auto src_hazard = access_context.DetectHazard(src_image, RangeFromLayers(region.srcSubresource), region.srcOffset,
                                                      region.extent, SYNC_COPY_TRANSFER_READ);
        if (src_hazard.IsHazard()) {
            const LogObjectList objlist = submit_time ? LogObjectList(env.handle, cb_context.GetCBState().Handle())
                                                      : LogObjectList(cb_context.GetCBState().Handle(), src_image.Handle());
            const std::string error = submit_time
                                          ? validator.error_messages_.SubmitTimeError(env, src_hazard, cb_context, replay_tag,
                                                                                      loc.index, validator.FormatHandle(src_image))
                                          : validator.error_messages_.ImageCopyResolveBlitError(
                                                env, src_hazard, loc.function, validator.FormatHandle(src_image),
                                                uint32_t(region_index), region.srcOffset, region.extent, region.srcSubresource);
            skip |= validator.SyncError(src_hazard.Hazard(), objlist, loc, error);
        }
        auto dst_hazard = access_context.DetectHazard(dst_image, RangeFromLayers(region.dstSubresource), region.dstOffset,
                                                      region.extent, SYNC_COPY_TRANSFER_WRITE);
        if (dst_hazard.IsHazard()) {
            const LogObjectList objlist = submit_time ? LogObjectList(env.handle, cb_context.GetCBState().Handle())
                                                      : LogObjectList(cb_context.GetCBState().Handle(), dst_image.Handle());
            const std::string error = submit_time
                                          ? validator.error_messages_.SubmitTimeError(env, dst_hazard, cb_context, replay_tag,
                                                                                      loc.index, validator.FormatHandle(dst_image))
                                          : validator.error_messages_.ImageCopyResolveBlitError(
                                                env, dst_hazard, loc.function, validator.FormatHandle(dst_image),
                                                uint32_t(region_index), region.dstOffset, region.extent, region.dstSubresource);
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
    for (const auto& image_barrier : barrier_set.image_barriers) {
        if (!image_barrier.layout_transition) {
            // The only accesses that originate from the pipeline barrier are layout transitions
            continue;
        }
        const vvl::Image& image_state = *image_barrier.image;
        const bool can_transition_depth_slices =
            CanTransitionDepthSlices(env.validator.extensions, image_state.GetImageType(), image_state.create_flags);

        const auto hazard = access_context.DetectImageBarrierHazard(
            image_state, image_barrier.barrier.src_exec_scope.exec_scope, image_barrier.barrier.src_access_scope,
            image_barrier.subresource_range, can_transition_depth_slices, AccessContext::kDetectAll, env.queue_id);

        if (hazard.IsHazard()) {
            if (replay_tag != kInvalidTag) {
                LogObjectList objlist(env.handle, cb_context.GetCBState().Handle());
                const std::string error = env.validator.error_messages_.SubmitTimeError(
                    env, hazard, cb_context, replay_tag, loc.index, env.validator.FormatHandle(image_state.Handle()));
                skip |= env.validator.SyncError(hazard.Hazard(), objlist, loc, error);
            } else {
                LogObjectList objlist(cb_context.GetCBState().Handle(), image_state.Handle());
                const std::string resource_description = env.validator.FormatHandle(image_state.Handle());
                const std::string error =
                    env.validator.error_messages_.ImageBarrierError(env, hazard, loc.function, resource_description, image_barrier);
                skip |= env.validator.SyncError(hazard.Hazard(), objlist, loc, error);
            }
        }
    }
    return skip;
}

void BarrierCommand::Apply(SyncEnvironment& env, ResourceUsageTag tag, AccessContext& access_context) const {
    ApplyBarrier(env, access_context, barrier_set, tag, true);
}

}  // namespace syncval
