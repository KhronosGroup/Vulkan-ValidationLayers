/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include "gpuav/validation_cmd/gpuav_validation_cmd_common.h"

#include <vulkan/vulkan_core.h>

#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_constants.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"

namespace gpuav {

namespace valcmd {
namespace internal {
static void BindErrorLoggingDescSet(Validator &gpuav, CommandBufferSubState &cb_state, VkPipelineBindPoint bind_point,
                                    VkPipelineLayout pipeline_layout, uint32_t cmd_index, uint32_t error_logger_index) {
    assert(cmd_index < cst::indices_count);
    assert(error_logger_index < cst::indices_count);
    std::array<uint32_t, 2> dynamic_offsets = {
        {cmd_index * gpuav.indices_buffer_alignment_, error_logger_index * gpuav.indices_buffer_alignment_}};

    DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, pipeline_layout, glsl::kDiagCommonDescriptorSet, 1,
                                  &cb_state.GetErrorLoggingDescSet(), static_cast<uint32_t>(dynamic_offsets.size()),
                                  dynamic_offsets.data());
}

void BindShaderResourcesHelper(Validator &gpuav, CommandBufferSubState &cb_state, uint32_t cmd_index, uint32_t error_logger_index,
                               VkPipelineLayout pipeline_layout, VkDescriptorSet desc_set,
                               const std::vector<VkWriteDescriptorSet> &descriptor_writes, const uint32_t push_constants_byte_size,
                               const void *push_constants) {
    // Error logging resources
    BindErrorLoggingDescSet(gpuav, cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, cmd_index, error_logger_index);

    // Any push constants byte size below 4 is illegal. Can come from empty push constant struct
    if (push_constants_byte_size >= 4) {
        DispatchCmdPushConstants(cb_state.VkHandle(), pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, push_constants_byte_size,
                                 push_constants);
    }

    if (!descriptor_writes.empty()) {
        // Specific resources
        DispatchUpdateDescriptorSets(gpuav.device, uint32_t(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);

        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, glsl::kValPipeDescSet,
                                      1, &desc_set, 0, nullptr);
    }
}
}  // namespace internal
}  // namespace valcmd

}  // namespace gpuav
