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
#include "sync/sync_validation.h"
#include "state_tracker/buffer_state.h"
#include "error_message/logging.h"

namespace syncval {

bool ReplayCommands(const SyncEnvironment& env, AccessContext& access_context, const CommandBufferContext& cb_context,
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
                    const ResourceUsageTagEx src_tag_ex{tag, storage.src_handle_index};
                    const ResourceUsageTagEx dst_tag_ex{tag, storage.dst_handle_index};
                    command.Apply(env, access_context, src_tag_ex, dst_tag_ex);
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

BufferCopyCommand BufferCopyCommand::Storage::MakeCommand(const CommandData& command_data) const {
    const vvl::Buffer& src_buffer = *command_data.buffers[src_buffer_index];
    const vvl::Buffer& dst_buffer = *command_data.buffers[dst_buffer_index];
    vvl::span<const BufferCopyRegion> regions;
    if (region_count != 0) {
        regions = vvl::make_span(&command_data.buffer_copy_regions[first_region], region_count);
    }
    return {src_buffer, dst_buffer, regions};
}

BufferCopyCommand::Storage BufferCopyCommand::MakeStorage(CommandData& command_data, uint32_t src_handle_index,
                                                          uint32_t dst_handle_index) const {
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

void BufferCopyCommand::Apply(const SyncEnvironment& env, AccessContext& access_context, ResourceUsageTagEx src_tag_ex,
                              ResourceUsageTagEx dst_tag_ex) const {
    for (const BufferCopyRegion& region : regions) {
        const AccessRange src_range = MakeRange(src_buffer, region.src_offset, region.size);
        access_context.UpdateAccessState(src_buffer, SYNC_COPY_TRANSFER_READ, src_range, src_tag_ex, 0, env.queue_id);

        const AccessRange dst_range = MakeRange(dst_buffer, region.dst_offset, region.size);
        access_context.UpdateAccessState(dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range, dst_tag_ex, 0, env.queue_id);
    }
}

}  // namespace syncval
