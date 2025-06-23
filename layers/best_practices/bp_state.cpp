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
    auto& secondary_sub_state = SubState(secondary_command_buffer);
    if (secondary_command_buffer.IsSecondary()) {
        render_pass_state.has_draw_cmd |= secondary_sub_state.render_pass_state.has_draw_cmd;
    }

    for (auto& function : secondary_sub_state.queue_submit_functions) {
        queue_submit_functions.push_back(function);
    }
}

void CommandBufferSubState::RecordPushConstants(VkPipelineLayout layout, VkShaderStageFlags stage_flags, uint32_t offset,
                                                uint32_t size, const void* values) {
    PushConstantData push_constant_data;
    push_constant_data.layout = layout;
    push_constant_data.stage_flags = stage_flags;
    push_constant_data.offset = offset;
    push_constant_data.values.resize(size);
    auto byte_values = static_cast<const std::byte*>(values);
    std::copy(byte_values, byte_values + size, push_constant_data.values.data());
    push_constant_data_chunks.emplace_back(push_constant_data);
}

void CommandBufferSubState::ClearPushConstants() { push_constant_data_chunks.clear(); }

void CommandBufferSubState::Destroy() { ResetCBState(); }

void CommandBufferSubState::Reset(const Location&) { ResetCBState(); }

void CommandBufferSubState::ResetCBState() {
    num_submits = 0;
    small_indexed_draw_call_count = 0;
    queue_submit_functions.clear();
    queue_submit_functions_after_render_pass.clear();
    ClearPushConstants();
}

void CommandBufferSubState::RecordCmd(vvl::Func command) {
    if (vvl::IsCommandDrawMesh(command) || vvl::IsCommandDrawVertex(command)) {
        render_pass_state.has_draw_cmd = true;
    }
}

void CommandBufferSubState::Submit(vvl::Queue& queue_state, uint32_t perf_submit_pass, const Location& loc) {
    for (auto& func : queue_submit_functions) {
        func(queue_state, base);
    }
}

}  // namespace bp_state