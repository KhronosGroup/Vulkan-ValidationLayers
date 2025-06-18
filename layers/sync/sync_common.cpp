/*
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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

#include "sync/sync_common.h"
#include "state_tracker/buffer_state.h"

const ResourceAccessRange kFullRange(0, std::numeric_limits<VkDeviceSize>::max());

ResourceAccessRange MakeRange(VkDeviceSize start, VkDeviceSize size) { return ResourceAccessRange(start, start + size); }

ResourceAccessRange MakeRange(const vvl::Buffer& buffer, VkDeviceSize offset, VkDeviceSize size) {
    if (offset >= buffer.create_info.size) {
        return {};
    }
    VkDeviceSize end;
    if (size == VK_WHOLE_SIZE) {
        end = buffer.create_info.size;
    } else {
        end = std::min(offset + size, buffer.create_info.size);
    }
    return ResourceAccessRange(offset, end);
}
