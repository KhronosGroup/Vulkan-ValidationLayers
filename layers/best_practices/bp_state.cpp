/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "best_practices/bp_state.h"
#include "utils/action_command_utils.h"

namespace bp_state {

static bool SparseMetaDataRequired(vvl::Image& img) {
    if (img.create_from_swapchain) {
        return false;
    }

    for (const auto& req : img.sparse_requirements) {
        if (req.formatProperties.aspectMask & VK_IMAGE_ASPECT_METADATA_BIT) {
            return true;
        }
    }
    return false;
}

ImageSubState::ImageSubState(vvl::Image& img) : vvl::ImageSubState(img), sparse_metadata_required(SparseMetaDataRequired(img)) {
    SetupUsages();
}

ImageSubState::Usage ImageSubState::UpdateUsage(uint32_t array_layer, uint32_t mip_level, IMAGE_SUBRESOURCE_USAGE_BP usage,
                                                uint32_t queue_family) {
    auto last_usage = usages_[array_layer][mip_level];
    usages_[array_layer][mip_level].type = usage;
    usages_[array_layer][mip_level].queue_family_index = queue_family;
    return last_usage;
}

ImageSubState::Usage ImageSubState::GetUsage(uint32_t array_layer, uint32_t mip_level) const {
    return usages_[array_layer][mip_level];
}

IMAGE_SUBRESOURCE_USAGE_BP ImageSubState::GetUsageType(uint32_t array_layer, uint32_t mip_level) const {
    return GetUsage(array_layer, mip_level).type;
}

uint32_t ImageSubState::GetLastQueueFamily(uint32_t array_layer, uint32_t mip_level) const {
    return GetUsage(array_layer, mip_level).queue_family_index;
}

void ImageSubState::SetupUsages() {
    usages_.resize(base.create_info.arrayLayers);
    for (auto& mip_vec : usages_) {
        mip_vec.resize(base.create_info.mipLevels, {IMAGE_SUBRESOURCE_USAGE_BP::UNDEFINED, VK_QUEUE_FAMILY_IGNORED});
    }
}

void CommandBufferSubState::ExecuteCommands(vvl::CommandBuffer& secondary_command_buffer) {
    if (secondary_command_buffer.IsSecondary()) {
        auto& secondary_sub_state = SubState(secondary_command_buffer);
        render_pass_state.has_draw_cmd |= secondary_sub_state.render_pass_state.has_draw_cmd;
    }
}

void CommandBufferSubState::RecordCmd(vvl::Func command) {
    if (vvl::IsCommandDrawMesh(command) || vvl::IsCommandDrawVertex(command)) {
        render_pass_state.has_draw_cmd = true;
    }
}

}  // namespace bp_state