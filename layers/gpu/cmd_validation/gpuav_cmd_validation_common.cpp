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
#include "gpu/core/gpuav_constants.h"
#include "gpu/shaders/gpu_shaders_constants.h"

#include "state_tracker/descriptor_sets.h"
#include "state_tracker/shader_object_state.h"

namespace gpuav {

void BindValidationCmdsCommonDescSet(Validator &gpuav, CommandBuffer &cb_state, VkPipelineBindPoint bind_point,
                                     VkPipelineLayout pipeline_layout, uint32_t cmd_index, uint32_t error_logger_index) {
    assert(cmd_index < cst::indices_count);
    assert(error_logger_index < cst::indices_count);
    std::array<uint32_t, 2> dynamic_offsets = {
        {cmd_index * gpuav.indices_buffer_alignment_, error_logger_index * gpuav.indices_buffer_alignment_}};

    DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, pipeline_layout, glsl::kDiagCommonDescriptorSet, 1,
                                  &cb_state.GetValidationCmdCommonDescriptorSet(), static_cast<uint32_t>(dynamic_offsets.size()),
                                  dynamic_offsets.data());
}

void RestorablePipelineState::Create(vvl::CommandBuffer &cb_state, VkPipelineBindPoint bind_point) {
    cmd_buffer_ = cb_state.VkHandle();
    pipeline_bind_point_ = bind_point;
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);

    LastBound &last_bound = cb_state.lastBound[lv_bind_point];
    if (last_bound.pipeline_state) {
        pipeline_ = last_bound.pipeline_state->VkHandle();

    } else {
        assert(shader_objects_.empty());
        if (lv_bind_point == BindPoint_Graphics) {
            shader_objects_ = last_bound.GetAllBoundGraphicsShaders();
        } else if (lv_bind_point == BindPoint_Compute) {
            auto compute_shader = last_bound.GetShaderState(ShaderObjectStage::COMPUTE);
            if (compute_shader) {
                shader_objects_.emplace_back(compute_shader);
            }
        }
    }

    desc_set_pipeline_layout_ = last_bound.desc_set_pipeline_layout;

    push_constants_data_ = cb_state.push_constant_data_chunks;

    descriptor_sets_.reserve(last_bound.per_set.size());
    for (std::size_t set_i = 0; set_i < last_bound.per_set.size(); set_i++) {
        const auto &bound_descriptor_set = last_bound.per_set[set_i].bound_descriptor_set;
        if (bound_descriptor_set) {
            descriptor_sets_.emplace_back(bound_descriptor_set->VkHandle(), static_cast<uint32_t>(set_i));
            if (bound_descriptor_set->IsPushDescriptor()) {
                push_descriptor_set_index_ = static_cast<uint32_t>(set_i);
            }
            dynamic_offsets_.push_back(last_bound.per_set[set_i].dynamicOffsets);
        }
    }

    if (last_bound.push_descriptor_set) {
        push_descriptor_set_writes_ = last_bound.push_descriptor_set->GetWrites();
    }
}

void RestorablePipelineState::Restore() const {
    if (pipeline_ != VK_NULL_HANDLE) {
        DispatchCmdBindPipeline(cmd_buffer_, pipeline_bind_point_, pipeline_);
    }
    if (!shader_objects_.empty()) {
        std::vector<VkShaderStageFlagBits> stages;
        std::vector<VkShaderEXT> shaders;
        for (const vvl::ShaderObject *shader_obj : shader_objects_) {
            stages.emplace_back(shader_obj->create_info.stage);
            shaders.emplace_back(shader_obj->VkHandle());
        }
        DispatchCmdBindShadersEXT(cmd_buffer_, static_cast<uint32_t>(shader_objects_.size()), stages.data(), shaders.data());
    }

    for (std::size_t i = 0; i < descriptor_sets_.size(); i++) {
        VkDescriptorSet descriptor_set = descriptor_sets_[i].first;
        if (descriptor_set != VK_NULL_HANDLE) {
            DispatchCmdBindDescriptorSets(cmd_buffer_, pipeline_bind_point_, desc_set_pipeline_layout_, descriptor_sets_[i].second,
                                          1, &descriptor_set, static_cast<uint32_t>(dynamic_offsets_[i].size()),
                                          dynamic_offsets_[i].data());
        }
    }

    if (!push_descriptor_set_writes_.empty()) {
        DispatchCmdPushDescriptorSetKHR(cmd_buffer_, pipeline_bind_point_, desc_set_pipeline_layout_, push_descriptor_set_index_,
                                        static_cast<uint32_t>(push_descriptor_set_writes_.size()),
                                        reinterpret_cast<const VkWriteDescriptorSet *>(push_descriptor_set_writes_.data()));
    }

    for (const auto &push_constant_range : push_constants_data_) {
        DispatchCmdPushConstants(cmd_buffer_, push_constant_range.layout, push_constant_range.stage_flags,
                                 push_constant_range.offset, static_cast<uint32_t>(push_constant_range.values.size()),
                                 push_constant_range.values.data());
    }
}

}  // namespace gpuav
