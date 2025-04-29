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

#include "gpuav/core/gpuav_validation_pipeline.h"

#include "generated/dispatch_functions.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/render_pass_state.h"

namespace gpuav {
namespace valpipe {
namespace internal {

bool CreateComputePipelineHelper(Validator &gpuav, const Location &loc,
                                 const std::vector<VkDescriptorSetLayoutBinding> specific_bindings,
                                 VkDescriptorSetLayout additional_desc_set_layout, uint32_t push_constants_byte_size,
                                 uint32_t spirv_size, const uint32_t *spirv, VkDevice &out_device,
                                 VkDescriptorSetLayout &out_specific_descriptor_set_layout, VkPipelineLayout &out_pipeline_layout,
                                 VkShaderModule &out_shader_module, VkPipeline &out_pipeline) {
    out_device = gpuav.device;
    VkPushConstantRange push_constant_range = {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = push_constants_byte_size;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();

    ds_layout_ci.bindingCount = static_cast<uint32_t>(specific_bindings.size());
    ds_layout_ci.pBindings = specific_bindings.data();
    VkResult result = DispatchCreateDescriptorSetLayout(gpuav.device, &ds_layout_ci, nullptr, &out_specific_descriptor_set_layout);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Failed to create descriptor set layout.");
        return false;
    }

    std::vector<VkDescriptorSetLayout> set_layouts = {out_specific_descriptor_set_layout};
    if (additional_desc_set_layout != VK_NULL_HANDLE) {
        set_layouts.emplace_back(additional_desc_set_layout);
    }
    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    // Any push constants byte size below 4 is illegal. Can come from empty push constant struct
    if (push_constant_range.size >= 4) {
        pipeline_layout_ci.pushConstantRangeCount = 1;
        pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
    }
    pipeline_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
    pipeline_layout_ci.pSetLayouts = set_layouts.data();
    result = DispatchCreatePipelineLayout(gpuav.device, &pipeline_layout_ci, nullptr, &out_pipeline_layout);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Failed to create pipeline layout.");
        return false;
    }

    VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
    shader_module_ci.codeSize = spirv_size;
    shader_module_ci.pCode = spirv;
    result = DispatchCreateShaderModule(gpuav.device, &shader_module_ci, nullptr, &out_shader_module);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Failed to create shader module.");
        return false;
    }

    VkComputePipelineCreateInfo compute_validation_pipeline_ci = vku::InitStructHelper();
    compute_validation_pipeline_ci.stage = vku::InitStructHelper();
    compute_validation_pipeline_ci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    compute_validation_pipeline_ci.stage.module = out_shader_module;
    compute_validation_pipeline_ci.stage.pName = "main";
    compute_validation_pipeline_ci.layout = out_pipeline_layout;
    result =
        DispatchCreateComputePipelines(gpuav.device, VK_NULL_HANDLE, 1, &compute_validation_pipeline_ci, nullptr, &out_pipeline);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Failed to create compute validation pipeline.");
        return false;
    }

    return true;
}

void DestroyComputePipelineHelper(VkDevice device, VkDescriptorSetLayout specific_descriptor_set_layout,
                                  VkPipelineLayout pipeline_layout, VkShaderModule shader_module, VkPipeline pipeline) {
    if (specific_descriptor_set_layout != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(device, specific_descriptor_set_layout, nullptr);
    }

    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(device, pipeline_layout, nullptr);
    }

    if (shader_module != VK_NULL_HANDLE) {
        DispatchDestroyShaderModule(device, shader_module, nullptr);
    }

    if (pipeline != VK_NULL_HANDLE) {
        DispatchDestroyPipeline(device, pipeline, nullptr);
    }
}

VkDescriptorSet GetDescriptorSetHelper(CommandBufferSubState &cb_state, VkDescriptorSetLayout desc_set_layout) {
    return cb_state.gpu_resources_manager.GetManagedDescriptorSet(desc_set_layout);
}

void BindShaderResourcesHelper(Validator &gpuav, CommandBufferSubState &cb_state, VkPipelineLayout pipeline_layout,
                               VkDescriptorSet desc_set, const std::vector<VkWriteDescriptorSet> &descriptor_writes,
                               const uint32_t push_constants_byte_size, const void *push_constants) {
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

void RestorablePipelineState::Create(CommandBufferSubState &cb_state, VkPipelineBindPoint bind_point) {
    pipeline_bind_point_ = bind_point;
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);

    LastBound &last_bound = cb_state.base.lastBound[lv_bind_point];
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

    desc_set_pipeline_layout_ =
        last_bound.desc_set_pipeline_layout ? last_bound.desc_set_pipeline_layout->VkHandle() : VK_NULL_HANDLE;

    push_constants_data_ = cb_state.base.push_constant_data_chunks;

    descriptor_sets_.reserve(last_bound.ds_slots.size());
    for (std::size_t set_i = 0; set_i < last_bound.ds_slots.size(); set_i++) {
        const auto &bound_descriptor_set = last_bound.ds_slots[set_i].ds_state;
        if (bound_descriptor_set) {
            descriptor_sets_.emplace_back(bound_descriptor_set->VkHandle(), static_cast<uint32_t>(set_i));
            if (bound_descriptor_set->IsPushDescriptor()) {
                push_descriptor_set_index_ = static_cast<uint32_t>(set_i);
            }
            dynamic_offsets_.push_back(last_bound.ds_slots[set_i].dynamic_offsets);
        }
    }

    if (last_bound.push_descriptor_set) {
        push_descriptor_set_writes_ = last_bound.push_descriptor_set->GetWrites();
    }

    // Do not handle cb_state.active_render_pass->use_dynamic_rendering_inherited for now
    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS && cb_state.base.active_render_pass &&
        cb_state.base.active_render_pass->use_dynamic_rendering) {
        rendering_info_ = &cb_state.base.active_render_pass->dynamic_rendering_begin_rendering_info;
        DispatchCmdEndRendering(cb_state.VkHandle());

        VkRenderingInfo rendering_info = vku::InitStructHelper();
        rendering_info.renderArea = {{0, 0}, {1, 1}};
        rendering_info.layerCount = 1;
        rendering_info.viewMask = 0;
        rendering_info.colorAttachmentCount = 0;
        DispatchCmdBeginRendering(cb_state.VkHandle(), &rendering_info);
    }
}

void RestorablePipelineState::Restore() const {
    if (rendering_info_) {
        DispatchCmdEndRendering(cb_state_.VkHandle());
        DispatchCmdBeginRendering(cb_state_.VkHandle(), rendering_info_->ptr());
    }

    if (pipeline_ != VK_NULL_HANDLE) {
        DispatchCmdBindPipeline(cb_state_.VkHandle(), pipeline_bind_point_, pipeline_);
    }
    if (!shader_objects_.empty()) {
        std::vector<VkShaderStageFlagBits> stages;
        std::vector<VkShaderEXT> shaders;
        for (const vvl::ShaderObject *shader_obj : shader_objects_) {
            stages.emplace_back(shader_obj->create_info.stage);
            shaders.emplace_back(shader_obj->VkHandle());
        }
        DispatchCmdBindShadersEXT(cb_state_.VkHandle(), static_cast<uint32_t>(shader_objects_.size()), stages.data(),
                                  shaders.data());
    }

    for (std::size_t i = 0; i < descriptor_sets_.size(); i++) {
        VkDescriptorSet descriptor_set = descriptor_sets_[i].first;
        if (descriptor_set != VK_NULL_HANDLE) {
            DispatchCmdBindDescriptorSets(cb_state_.VkHandle(), pipeline_bind_point_, desc_set_pipeline_layout_,
                                          descriptor_sets_[i].second, 1, &descriptor_set,
                                          static_cast<uint32_t>(dynamic_offsets_[i].size()), dynamic_offsets_[i].data());
        }
    }

    if (!push_descriptor_set_writes_.empty()) {
        DispatchCmdPushDescriptorSetKHR(cb_state_.VkHandle(), pipeline_bind_point_, desc_set_pipeline_layout_,
                                        push_descriptor_set_index_, static_cast<uint32_t>(push_descriptor_set_writes_.size()),
                                        reinterpret_cast<const VkWriteDescriptorSet *>(push_descriptor_set_writes_.data()));
    }

    for (const auto &push_constant_range : push_constants_data_) {
        DispatchCmdPushConstants(cb_state_.VkHandle(), push_constant_range.layout, push_constant_range.stage_flags,
                                 push_constant_range.offset, static_cast<uint32_t>(push_constant_range.values.size()),
                                 push_constant_range.values.data());
    }
}
}  // namespace valpipe
}  // namespace gpuav
