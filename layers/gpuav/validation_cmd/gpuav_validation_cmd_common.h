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

#pragma once

#include <vector>
#include "gpuav/core/gpuav_validation_pipeline.h"
#include "gpuav/resources/gpuav_state_trackers.h"

struct Location;
namespace gpuav {
namespace valcmd {

namespace internal {
void BindShaderResourcesHelper(Validator& gpuav, CommandBufferSubState& cb_state, uint32_t cmd_index, uint32_t error_logger_index,
                               VkPipelineLayout pipeline_layout, VkDescriptorSet desc_set,
                               const std::vector<VkWriteDescriptorSet>& descriptor_writes, const uint32_t push_constants_byte_size,
                               const void* push_constants, bool bind_error_logging_desc_set);
}

template <typename ShaderResources>
[[nodiscard]] bool BindShaderResources(gpuav::valpipe::ComputePipeline<ShaderResources>& validation_pipeline, Validator& gpuav,
                                       CommandBufferSubState& cb_state, uint32_t cmd_index, uint32_t error_logger_index,
                                       const ShaderResources& shader_resources, bool bind_error_logging_desc_set = true) {
    std::vector<VkWriteDescriptorSet> desc_writes = shader_resources.GetDescriptorWrites();
    VkDescriptorSet desc_set = VK_NULL_HANDLE;
    if (!desc_writes.empty()) {
        desc_set = cb_state.gpu_resources_manager.GetManagedDescriptorSet(validation_pipeline.specific_desc_set_layout);
        if (!desc_set) {
            return false;
        }
    }
    for (VkWriteDescriptorSet& wds : desc_writes) {
        wds.dstSet = desc_set;
    }

    internal::BindShaderResourcesHelper(gpuav, cb_state, cmd_index, error_logger_index, validation_pipeline.pipeline_layout,
                                        desc_set, desc_writes, sizeof(shader_resources.push_constants),
                                        &shader_resources.push_constants, bind_error_logging_desc_set);
    return true;
}

class ValidationCommandsCommon {
  public:
    ValidationCommandsCommon(Validator& gpuav, CommandBufferSubState& cb, const Location& loc);
    ~ValidationCommandsCommon();

    VkDescriptorSetLayout error_logging_desc_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorSet error_logging_desc_set_ = VK_NULL_HANDLE;
    VkDescriptorPool validation_cmd_desc_pool_ = VK_NULL_HANDLE;

  private:
    Validator& gpuav_;
};
}  // namespace valcmd

}  // namespace gpuav
