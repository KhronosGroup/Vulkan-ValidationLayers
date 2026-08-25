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

bool BufferCopyCommand::Validate(const SyncEnvironment& env, const AccessContext& access_context, const Location& loc,
                                 VulkanTypedHandle command_buffer_handle) const {
    bool skip = false;
    const SyncValidator& validator = env.validator;

    for (const auto [region_index, copy_region] : vvl::enumerate(regions)) {
        const AccessRange src_range = MakeRange(src_buffer, copy_region.src_offset, copy_region.size);
        auto src_hazard = access_context.DetectHazard(src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
        if (src_hazard.IsHazard()) {
            const LogObjectList objlist(command_buffer_handle, src_buffer.Handle());
            const std::string error = validator.error_messages_.BufferCopyError(
                env, src_hazard, loc.function, validator.FormatHandle(src_buffer), uint32_t(region_index), src_range);
            skip |= validator.SyncError(src_hazard.Hazard(), objlist, loc, error);
        }
        const AccessRange dst_range = MakeRange(dst_buffer, copy_region.dst_offset, copy_region.size);
        auto dst_hazard = access_context.DetectHazard(dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
        if (dst_hazard.IsHazard()) {
            const LogObjectList objlist(command_buffer_handle, dst_buffer.Handle());
            const std::string error = validator.error_messages_.BufferCopyError(
                env, dst_hazard, loc.function, validator.FormatHandle(dst_buffer), uint32_t(region_index), dst_range);
            skip |= validator.SyncError(dst_hazard.Hazard(), objlist, loc, error);
        }
        if (skip) {
            break;
        }
    }
    return skip;
}

}  // namespace syncval
