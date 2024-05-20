/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
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

#include "gpu/cmd_validation/gpuav_cmd_validation_common.h"

#include "gpu/core/gpuav.h"
#include "gpu/resources/gpuav_resources.h"

#include "state_tracker/descriptor_sets.h"
#include "state_tracker/shader_object_state.h"

namespace gpuav {

void RestorablePipelineState::Create(vvl::CommandBuffer &cb_state, VkPipelineBindPoint bind_point) {
    pipeline_bind_point = bind_point;
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);

    LastBound &last_bound = cb_state.lastBound[lv_bind_point];
    if (last_bound.pipeline_state) {
        pipeline = last_bound.pipeline_state->VkHandle();
        pipeline_layout = last_bound.pipeline_layout;
        descriptor_sets.reserve(last_bound.per_set.size());
        for (std::size_t i = 0; i < last_bound.per_set.size(); i++) {
            const auto &bound_descriptor_set = last_bound.per_set[i].bound_descriptor_set;
            if (bound_descriptor_set) {
                descriptor_sets.push_back(std::make_pair(bound_descriptor_set->VkHandle(), static_cast<uint32_t>(i)));
                if (bound_descriptor_set->IsPushDescriptor()) {
                    push_descriptor_set_index = static_cast<uint32_t>(i);
                }
                dynamic_offsets.push_back(last_bound.per_set[i].dynamicOffsets);
            }
        }

        if (last_bound.push_descriptor_set) {
            push_descriptor_set_writes = last_bound.push_descriptor_set->GetWrites();
        }
        const auto &pipeline_layout = last_bound.pipeline_state->PipelineLayoutState();
        if (pipeline_layout->push_constant_ranges == cb_state.push_constant_data_ranges) {
            push_constants_data = cb_state.push_constant_data;
            push_constants_ranges = pipeline_layout->push_constant_ranges;
        }
    } else {
        assert(shader_objects.empty());
        if (lv_bind_point == BindPoint_Graphics) {
            shader_objects = last_bound.GetAllBoundGraphicsShaders();
        } else if (lv_bind_point == BindPoint_Compute) {
            auto compute_shader = last_bound.GetShaderState(ShaderObjectStage::COMPUTE);
            if (compute_shader) {
                shader_objects.emplace_back(compute_shader);
            }
        }
    }
}

void RestorablePipelineState::Restore(VkCommandBuffer command_buffer) const {
    if (pipeline != VK_NULL_HANDLE) {
        DispatchCmdBindPipeline(command_buffer, pipeline_bind_point, pipeline);
        if (!descriptor_sets.empty()) {
            for (std::size_t i = 0; i < descriptor_sets.size(); i++) {
                VkDescriptorSet descriptor_set = descriptor_sets[i].first;
                if (descriptor_set != VK_NULL_HANDLE) {
                    DispatchCmdBindDescriptorSets(command_buffer, pipeline_bind_point, pipeline_layout, descriptor_sets[i].second,
                                                  1, &descriptor_set, static_cast<uint32_t>(dynamic_offsets[i].size()),
                                                  dynamic_offsets[i].data());
                }
            }
        }
        if (!push_descriptor_set_writes.empty()) {
            DispatchCmdPushDescriptorSetKHR(command_buffer, pipeline_bind_point, pipeline_layout, push_descriptor_set_index,
                                            static_cast<uint32_t>(push_descriptor_set_writes.size()),
                                            reinterpret_cast<const VkWriteDescriptorSet *>(push_descriptor_set_writes.data()));
        }
        if (!push_constants_data.empty()) {
            for (const auto &push_constant_range : *push_constants_ranges) {
                if (push_constant_range.size == 0) continue;
                DispatchCmdPushConstants(command_buffer, pipeline_layout, push_constant_range.stageFlags,
                                         push_constant_range.offset, push_constant_range.size, push_constants_data.data());
            }
        }
    }
    if (!shader_objects.empty()) {
        std::vector<VkShaderStageFlagBits> stages;
        std::vector<VkShaderEXT> shaders;
        for (const vvl::ShaderObject *shader_obj : shader_objects) {
            stages.emplace_back(shader_obj->create_info.stage);
            shaders.emplace_back(shader_obj->VkHandle());
        }
        DispatchCmdBindShadersEXT(command_buffer, static_cast<uint32_t>(shader_objects.size()), stages.data(), shaders.data());
    }
}

void CommandResources::Destroy(Validator &validator) {
    if (instrumentation_desc_set != VK_NULL_HANDLE) {
        validator.desc_set_manager->PutBackDescriptorSet(instrumentation_desc_pool, instrumentation_desc_set);
        instrumentation_desc_set = VK_NULL_HANDLE;
        instrumentation_desc_pool = VK_NULL_HANDLE;
    }
}

}  // namespace gpuav
