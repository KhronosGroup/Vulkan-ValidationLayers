/* Copyright (c) 2018-2026 The Khronos Group Inc.
 * Copyright (c) 2018-2026 Valve Corporation
 * Copyright (c) 2018-2026 LunarG, Inc.
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
#include <cstdint>
#include <memory>

#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "vulkan/utility/vk_struct_helper.hpp"
#include "vulkan/vulkan_core.h"

namespace gpuav {

namespace valcmd {

ValidationCommandsGpuavState::ValidationCommandsGpuavState(Validator& gpuav, const Location& loc) : gpuav_(gpuav) {
    validation_cmd_bindings = {{
        // Error output buffer
        {glsl::kBindingDiagErrorBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding action command index in command buffer
        {glsl::kBindingDiagActionIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding a resource index from the per command buffer command resources list
        {glsl::kBindingDiagCmdResourceIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Commands errors counts buffer
        {glsl::kBindingDiagCmdErrorsCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    }};

    VkDescriptorSetLayoutCreateInfo validation_cmd_desc_set_layout_ci = vku::InitStructHelper();
    validation_cmd_desc_set_layout_ci.bindingCount = static_cast<uint32_t>(validation_cmd_bindings.size());
    validation_cmd_desc_set_layout_ci.pBindings = validation_cmd_bindings.data();
    const VkResult result = DispatchCreateDescriptorSetLayout(gpuav_.device, &validation_cmd_desc_set_layout_ci, nullptr,
                                                              &desc_set_mode.error_logging_desc_set_layout_);
    if (result != VK_SUCCESS) {
        gpuav_.InternalError(gpuav_.device, loc, "Unable to create descriptor set layout used for validation commands.");
        return;
    }
}

ValidationCommandsGpuavState::~ValidationCommandsGpuavState() {
    if (desc_set_mode.error_logging_desc_set_layout_ != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(gpuav_.device, desc_set_mode.error_logging_desc_set_layout_, nullptr);
    }
}

ValidationCommandsCbState::ValidationCommandsCbState(Validator& gpuav, CommandBufferSubState& cb,
                                                     VkDescriptorSetLayout error_logging_desc_set_layout, const Location& loc)
    : gpuav_(gpuav) {
    assert((validation_cmd_desc_pool_ == VK_NULL_HANDLE) == (error_logging_desc_set_ == VK_NULL_HANDLE));
    if (validation_cmd_desc_pool_ == VK_NULL_HANDLE && error_logging_desc_set_ == VK_NULL_HANDLE) {
        const VkResult result = gpuav_.desc_set_manager_->GetDescriptorSet(&validation_cmd_desc_pool_,
                                                                           error_logging_desc_set_layout, &error_logging_desc_set_);
        if (result != VK_SUCCESS) {
            gpuav_.InternalError(gpuav_.device, loc, "Unable to create descriptor set used for validation commands.");
            return;
        }
    }

    std::array<VkWriteDescriptorSet, 4> validation_cmd_descriptor_writes = {};

    VkDescriptorBufferInfo error_output_buffer_desc_info = {};

    assert(cb.error_output_buffer_range_.buffer != VK_NULL_HANDLE);
    error_output_buffer_desc_info.buffer = cb.error_output_buffer_range_.buffer;
    error_output_buffer_desc_info.offset = cb.error_output_buffer_range_.offset;
    error_output_buffer_desc_info.range = cb.error_output_buffer_range_.size;

    validation_cmd_descriptor_writes[0] = vku::InitStructHelper();
    validation_cmd_descriptor_writes[0].dstBinding = glsl::kBindingDiagErrorBuffer;
    validation_cmd_descriptor_writes[0].descriptorCount = 1;
    validation_cmd_descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    validation_cmd_descriptor_writes[0].pBufferInfo = &error_output_buffer_desc_info;
    validation_cmd_descriptor_writes[0].dstSet = error_logging_desc_set_;

    VkDescriptorBufferInfo cmd_indices_buffer_desc_info = {};

    assert(!gpuav_.global_indices_buffer_.IsDestroyed());
    cmd_indices_buffer_desc_info.buffer = gpuav_.global_indices_buffer_.VkHandle();
    cmd_indices_buffer_desc_info.offset = 0;
    cmd_indices_buffer_desc_info.range = sizeof(uint32_t);

    validation_cmd_descriptor_writes[1] = vku::InitStructHelper();
    validation_cmd_descriptor_writes[1].dstBinding = glsl::kBindingDiagActionIndex;
    validation_cmd_descriptor_writes[1].descriptorCount = 1;
    validation_cmd_descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    validation_cmd_descriptor_writes[1].pBufferInfo = &cmd_indices_buffer_desc_info;
    validation_cmd_descriptor_writes[1].dstSet = error_logging_desc_set_;

    validation_cmd_descriptor_writes[2] = validation_cmd_descriptor_writes[1];
    validation_cmd_descriptor_writes[2].dstBinding = glsl::kBindingDiagCmdResourceIndex;

    VkDescriptorBufferInfo cmd_errors_count_buffer_desc_info = {};
    cmd_errors_count_buffer_desc_info.buffer = cb.GetCmdErrorsCountsBuffer().VkHandle();
    cmd_errors_count_buffer_desc_info.offset = 0;
    cmd_errors_count_buffer_desc_info.range = VK_WHOLE_SIZE;

    validation_cmd_descriptor_writes[3] = vku::InitStructHelper();
    validation_cmd_descriptor_writes[3].dstBinding = glsl::kBindingDiagCmdErrorsCount;
    validation_cmd_descriptor_writes[3].descriptorCount = 1;
    validation_cmd_descriptor_writes[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    validation_cmd_descriptor_writes[3].pBufferInfo = &cmd_errors_count_buffer_desc_info;
    validation_cmd_descriptor_writes[3].dstSet = error_logging_desc_set_;

    DispatchUpdateDescriptorSets(gpuav_.device, static_cast<uint32_t>(validation_cmd_descriptor_writes.size()),
                                 validation_cmd_descriptor_writes.data(), 0, NULL);
}

ValidationCommandsCbState::~ValidationCommandsCbState() {
    if (validation_cmd_desc_pool_ != VK_NULL_HANDLE && error_logging_desc_set_ != VK_NULL_HANDLE) {
        gpuav_.desc_set_manager_->PutBackDescriptorSet(validation_cmd_desc_pool_, error_logging_desc_set_);
    }
}
}  // namespace valcmd

}  // namespace gpuav
