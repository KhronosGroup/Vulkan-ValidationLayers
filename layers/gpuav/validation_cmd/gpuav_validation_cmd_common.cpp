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
#include "gpuav/shaders/root_node.h"

namespace gpuav {

namespace valcmd {
namespace internal {
static void BindErrorLoggingDescSet(Validator &gpuav, CommandBufferSubState &cb_state, VkPipelineBindPoint bind_point,
                                    VkPipelineLayout pipeline_layout, uint32_t cmd_index, uint32_t error_logger_index) {
    assert(cmd_index < cst::indices_count);
    assert(error_logger_index < cst::indices_count);

    vko::BufferRange root_node_struct_buffer_range =
        cb_state.gpu_resources_manager.GetHostVisibleBufferRange(sizeof(glsl::RootNode));
    vko::BufferRange root_node_ptr_buffer_range = cb_state.gpu_resources_manager.GetHostVisibleBufferRange(sizeof(VkDeviceAddress));
    *(VkDeviceAddress *)root_node_ptr_buffer_range.offset_mapped_ptr = root_node_struct_buffer_range.offset_address;
    auto root_node_ptr = static_cast<glsl::RootNode *>(root_node_struct_buffer_range.offset_mapped_ptr);

    // Error output buffer
    root_node_ptr->inst_errors_buffer = cb_state.GetErrorOutputBufferRange().offset_address;
    assert(root_node_ptr->inst_errors_buffer);

    // Buffer holding action command index in command buffer
    root_node_ptr->inst_action_index_buffer = gpuav.indices_buffer_.Address() + cmd_index * gpuav.indices_buffer_alignment_;
    assert(root_node_ptr->inst_action_index_buffer);

    // Buffer holding a resource index from the per command buffer command resources list
    root_node_ptr->inst_error_logger_index_buffer =
        gpuav.indices_buffer_.Address() + error_logger_index * gpuav.indices_buffer_alignment_;
    assert(root_node_ptr->inst_error_logger_index_buffer);

    // Errors count buffer
    root_node_ptr->inst_cmd_errors_count_buffer = cb_state.GetCmdErrorsCountsBuffer().Address();
    assert(root_node_ptr->inst_cmd_errors_count_buffer);

    ValidationCommandsCommon &val_cmd_common = cb_state.shared_resources_cache.Get<ValidationCommandsCommon>();
    VkDescriptorSet val_cmd_common_desc_set =
        cb_state.gpu_resources_manager.GetManagedDescriptorSet(val_cmd_common.error_logging_desc_set_layout_);

    VkDescriptorBufferInfo dbi;
    dbi.buffer = root_node_ptr_buffer_range.buffer;
    dbi.offset = root_node_ptr_buffer_range.offset;
    dbi.range = root_node_ptr_buffer_range.size;
    VkWriteDescriptorSet wds = vku::InitStructHelper();
    wds.dstSet = val_cmd_common_desc_set;
    wds.dstBinding = glsl::kBindingInstRootNode;
    wds.descriptorCount = 1;
    wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    wds.pBufferInfo = &dbi;

    DispatchUpdateDescriptorSets(gpuav.device, 1, &wds, 0, nullptr);

    DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, pipeline_layout, glsl::kValPipeDescSet, 1,
                                  &val_cmd_common_desc_set, 0, nullptr);
}

void BindShaderResourcesHelper(Validator &gpuav, CommandBufferSubState &cb_state, uint32_t cmd_index, uint32_t error_logger_index,
                               VkPipelineLayout pipeline_layout, const uint32_t push_constants_byte_size,
                               const void *push_constants) {
    // Error logging resources
    BindErrorLoggingDescSet(gpuav, cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, cmd_index, error_logger_index);

    // Any push constants byte size below 4 is illegal. Can come from empty push constant struct
    if (push_constants_byte_size >= 4) {
        DispatchCmdPushConstants(cb_state.VkHandle(), pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, push_constants_byte_size,
                                 push_constants);
    }
}
}  // namespace internal

ValidationCommandsCommon::ValidationCommandsCommon(Validator &gpuav, CommandBufferSubState &cb, const Location &loc)
    : gpuav_(gpuav) {
    const std::vector<VkDescriptorSetLayoutBinding> validation_cmd_bindings = {
        // Root Node Address
        {glsl::kBindingInstRootNode, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };

    if (error_logging_desc_set_layout_ == VK_NULL_HANDLE) {
        VkDescriptorSetLayoutCreateInfo validation_cmd_desc_set_layout_ci = vku::InitStructHelper();
        validation_cmd_desc_set_layout_ci.bindingCount = static_cast<uint32_t>(validation_cmd_bindings.size());
        validation_cmd_desc_set_layout_ci.pBindings = validation_cmd_bindings.data();
        const VkResult result = DispatchCreateDescriptorSetLayout(gpuav_.device, &validation_cmd_desc_set_layout_ci, nullptr,
                                                                  &error_logging_desc_set_layout_);
        if (result != VK_SUCCESS) {
            gpuav_.InternalError(gpuav_.device, loc, "Unable to create descriptor set layout used for validation commands.");
            return;
        }
    }
}

ValidationCommandsCommon::~ValidationCommandsCommon() {
    if (error_logging_desc_set_layout_ != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(gpuav_.device, error_logging_desc_set_layout_, nullptr);
        error_logging_desc_set_layout_ = VK_NULL_HANDLE;
    }
}
}  // namespace valcmd

}  // namespace gpuav
