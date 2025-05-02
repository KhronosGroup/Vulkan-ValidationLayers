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

#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_validation_pipeline.h"
#include "gpuav/validation_cmd/gpuav_validation_cmd_common.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/shaders/validation_cmd/push_data.h"
#include "generated/gpuav_offline_spirv.h"

namespace gpuav {
namespace valcmd {

struct DispatchValidationShader {
    static size_t GetSpirvSize() { return validation_cmd_dispatch_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return validation_cmd_dispatch_comp; }

    glsl::DispatchPushData push_constants{};
    valpipe::BoundStorageBuffer indirect_buffer_binding = {glsl::kPreDispatchBinding_DispatchIndirectBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {glsl::kPreDispatchBinding_DispatchIndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},  // indirect buffer

        };

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) const {
        std::vector<VkWriteDescriptorSet> desc_writes(1);

        desc_writes[0] = vku::InitStructHelper();
        desc_writes[0].dstSet = desc_set;
        desc_writes[0].dstBinding = indirect_buffer_binding.binding;
        desc_writes[0].dstArrayElement = 0;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[0].pBufferInfo = &indirect_buffer_binding.info;

        return desc_writes;
    }
};

void DispatchIndirect(Validator &gpuav, const Location &loc, CommandBufferSubState &cb_state, VkBuffer indirect_buffer,
                      VkDeviceSize indirect_offset) {
    if (!gpuav.gpuav_settings.validate_indirect_dispatches_buffers) {
        return;
    }

    if (cb_state.max_actions_cmd_validation_reached_) {
        return;
    }

    valpipe::ComputePipeline<DispatchValidationShader> &validation_pipeline =
        gpuav.shared_resources_manager.GetOrCreate<valpipe::ComputePipeline<DispatchValidationShader>>(
            gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
    if (!validation_pipeline.valid) {
        return;
    }

    valpipe::RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    // Setup shader resources
    // ---
    {
        DispatchValidationShader shader_resources;
        shader_resources.push_constants.limit_x = gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0];
        shader_resources.push_constants.limit_y = gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[1];
        shader_resources.push_constants.limit_z = gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[2];
        shader_resources.push_constants.indirect_x_offset = static_cast<uint32_t>((indirect_offset / sizeof(uint32_t)));

        shader_resources.indirect_buffer_binding.info = {indirect_buffer, 0, VK_WHOLE_SIZE};

        if (!BindShaderResources(validation_pipeline, gpuav, cb_state, cb_state.compute_index,
                                 uint32_t(cb_state.per_command_error_loggers.size()), shader_resources)) {
            return;
        }
    }

    // Setup validation pipeline
    // ---
    {
        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

        DispatchCmdDispatch(cb_state.VkHandle(), 1, 1, 1);
    }

    CommandBufferSubState::ErrorLoggerFunc error_logger = [&gpuav, loc](const uint32_t *error_record, const LogObjectList &objlist,
                                                                        const std::vector<std::string> &) {
        bool skip = false;
        using namespace glsl;

        const uint32_t error_group = error_record[kHeaderShaderIdErrorOffset] >> kErrorGroupShift;
        if (error_group != kErrorGroupGpuPreDispatch) {
            return skip;
        }

        const uint32_t error_sub_code = (error_record[kHeaderShaderIdErrorOffset] & kErrorSubCodeMask) >> kErrorSubCodeShift;
        switch (error_sub_code) {
            case kErrorSubCodePreDispatchCountLimitX: {
                uint32_t count = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError("VUID-VkDispatchIndirectCommand-x-00417", objlist, loc,
                                       "Indirect dispatch VkDispatchIndirectCommand::x of %" PRIu32
                                       " would exceed maxComputeWorkGroupCount[0] limit of %" PRIu32 ".",
                                       count, gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0]);
                break;
            }
            case kErrorSubCodePreDispatchCountLimitY: {
                uint32_t count = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError("VUID-VkDispatchIndirectCommand-y-00418", objlist, loc,
                                       "Indirect dispatch VkDispatchIndirectCommand::y of %" PRIu32
                                       " would exceed maxComputeWorkGroupCount[1] limit of %" PRIu32 ".",
                                       count, gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[1]);
                break;
            }
            case kErrorSubCodePreDispatchCountLimitZ: {
                uint32_t count = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError("VUID-VkDispatchIndirectCommand-z-00419", objlist, loc,
                                       "Indirect dispatch VkDispatchIndirectCommand::z of %" PRIu32
                                       " would exceed maxComputeWorkGroupCount[2] limit of %" PRIu32 ".",
                                       count, gpuav.phys_dev_props.limits.maxComputeWorkGroupCount[0]);
                break;
            }
            default:
                break;
        }

        return skip;
    };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}
}  // namespace valcmd
}  // namespace gpuav
