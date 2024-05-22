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

#include "gpu/core/gpuav.h"
#include "gpu/cmd_validation/gpuav_cmd_validation_common.h"
#include "gpu/resources/gpuav_subclasses.h"
// Generated shaders
#include "generated/cmd_validation_dispatch_comp.h"

namespace gpuav {

std::unique_ptr<CommandResources> Validator::AllocatePreDispatchIndirectValidationResources(const Location &loc,
                                                                                            VkCommandBuffer cmd_buffer,
                                                                                            VkBuffer indirect_buffer,
                                                                                            VkDeviceSize indirect_offset) {
    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        InternalError(cmd_buffer, loc, "Unrecognized command buffer");
        return nullptr;
    }

    if (!gpuav_settings.validate_indirect_dispatches_buffers) {
        CommandResources cmd_resources = SetupShaderInstrumentationResources(cb_node, VK_PIPELINE_BIND_POINT_COMPUTE, loc);
        auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
        return cmd_resources_ptr;
    }

    // Insert a dispatch that can examine some device memory right before the dispatch we're validating
    //
    // NOTE that this validation does not attempt to abort invalid api calls as most other validation does. A crash
    // or DEVICE_LOST resulting from the invalid call will prevent preceding validation errors from being reported.
    auto dispatch_resources = std::make_unique<PreDispatchResources>();

    {
        const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_COMPUTE);
        auto const &last_bound = cb_node->lastBound[lv_bind_point];
        const auto *pipeline_state = last_bound.pipeline_state;
        const bool use_shader_objects = pipeline_state == nullptr;

        PreDispatchResources::SharedResources *shared_resources = GetSharedDispatchIndirectValidationResources(
            cb_node->GetValidationCmdCommonDescriptorSetLayout(), use_shader_objects, loc);
        if (!shared_resources) {
            return std::make_unique<PreDispatchResources>();
        }

        dispatch_resources->indirect_buffer = indirect_buffer;
        dispatch_resources->indirect_buffer_offset = indirect_offset;

        VkResult result = VK_SUCCESS;
        result = desc_set_manager_->GetDescriptorSet(&dispatch_resources->desc_pool, shared_resources->ds_layout,
                                                     &dispatch_resources->indirect_buffer_desc_set);
        if (result != VK_SUCCESS) {
            InternalError(cmd_buffer, loc, "Unable to allocate descriptor set. Aborting GPU-AV");
            return nullptr;
        }

        VkDescriptorBufferInfo desc_buffer_info{};
        // Indirect buffer
        desc_buffer_info.buffer = indirect_buffer;
        desc_buffer_info.offset = 0;
        desc_buffer_info.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet desc_write{};
        desc_write = vku::InitStructHelper();
        desc_write.dstBinding = 0;
        desc_write.descriptorCount = 1;
        desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_write.pBufferInfo = &desc_buffer_info;
        desc_write.dstSet = dispatch_resources->indirect_buffer_desc_set;

        DispatchUpdateDescriptorSets(device, 1, &desc_write, 0, nullptr);

        // Save current graphics pipeline state
        RestorablePipelineState restorable_state(*cb_node, VK_PIPELINE_BIND_POINT_COMPUTE);

        // Insert diagnostic dispatch
        if (use_shader_objects) {
            VkShaderStageFlagBits stage = VK_SHADER_STAGE_COMPUTE_BIT;
            DispatchCmdBindShadersEXT(cmd_buffer, 1u, &stage, &shared_resources->shader_object);
        } else {
            DispatchCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shared_resources->pipeline);
        }
        uint32_t push_constants[PreDispatchResources::push_constant_words] = {};
        push_constants[0] = phys_dev_props.limits.maxComputeWorkGroupCount[0];
        push_constants[1] = phys_dev_props.limits.maxComputeWorkGroupCount[1];
        push_constants[2] = phys_dev_props.limits.maxComputeWorkGroupCount[2];
        push_constants[3] = static_cast<uint32_t>((indirect_offset / sizeof(uint32_t)));
        DispatchCmdPushConstants(cmd_buffer, shared_resources->pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                 sizeof(push_constants), push_constants);
        BindValidationCmdsCommonDescSet(cb_node, VK_PIPELINE_BIND_POINT_COMPUTE, shared_resources->pipeline_layout,
                                        cb_node->compute_index, static_cast<uint32_t>(cb_node->per_command_resources.size()));
        DispatchCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shared_resources->pipeline_layout,
                                      glsl::kDiagPerCmdDescriptorSet, 1, &dispatch_resources->indirect_buffer_desc_set, 0, nullptr);
        DispatchCmdDispatch(cmd_buffer, 1, 1, 1);

        CommandResources cmd_resources = SetupShaderInstrumentationResources(cb_node, VK_PIPELINE_BIND_POINT_COMPUTE, loc);
        if (aborted_) {
            return nullptr;
        }
        CommandResources &base = *dispatch_resources;
        base = cmd_resources;

        // Restore the previous compute pipeline state.
        restorable_state.Restore(cmd_buffer);
    }

    return dispatch_resources;
}

PreDispatchResources::SharedResources *Validator::GetSharedDispatchIndirectValidationResources(
    VkDescriptorSetLayout error_output_desc_set, bool use_shader_objects, const Location &loc) {
    if (auto shared_resources = shared_validation_resources_map.find(typeid(PreDispatchResources::SharedResources));
        shared_resources != shared_validation_resources_map.end()) {
        return reinterpret_cast<PreDispatchResources::SharedResources *>(shared_resources->second.get());
    }

    auto shared_resources = std::make_unique<PreDispatchResources::SharedResources>();

    VkResult result = VK_SUCCESS;
    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},  // indirect buffer
    };

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
    ds_layout_ci.pBindings = bindings.data();
    result = DispatchCreateDescriptorSetLayout(device, &ds_layout_ci, nullptr, &shared_resources->ds_layout);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Unable to create descriptor set layout. Aborting GPU-AV");
        return nullptr;
    }

    VkPushConstantRange push_constant_range = {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = PreDispatchResources::push_constant_words * sizeof(uint32_t);

    std::array<VkDescriptorSetLayout, 2> set_layouts = {{error_output_desc_set, shared_resources->ds_layout}};
    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pushConstantRangeCount = 1;
    pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
    pipeline_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
    pipeline_layout_ci.pSetLayouts = set_layouts.data();
    result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &shared_resources->pipeline_layout);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Unable to create pipeline layout. Aborting GPU-AV");
        return nullptr;
    }

    if (use_shader_objects) {
        VkShaderCreateInfoEXT shader_ci = vku::InitStructHelper();
        shader_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shader_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        shader_ci.codeSize = cmd_validation_dispatch_comp_size * sizeof(uint32_t);
        shader_ci.pCode = cmd_validation_dispatch_comp;
        shader_ci.pName = "main";
        shader_ci.setLayoutCount = pipeline_layout_ci.setLayoutCount;
        shader_ci.pSetLayouts = pipeline_layout_ci.pSetLayouts;
        shader_ci.pushConstantRangeCount = pipeline_layout_ci.pushConstantRangeCount;
        shader_ci.pPushConstantRanges = pipeline_layout_ci.pPushConstantRanges;
        result = DispatchCreateShadersEXT(device, 1u, &shader_ci, nullptr, &shared_resources->shader_object);
        if (result != VK_SUCCESS) {
            InternalError(device, loc, "Unable to create shader object. Aborting GPU-AV");
            return nullptr;
        }
    } else {
        VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
        shader_module_ci.codeSize = cmd_validation_dispatch_comp_size * sizeof(uint32_t);
        shader_module_ci.pCode = cmd_validation_dispatch_comp;
        VkShaderModule validation_shader = VK_NULL_HANDLE;
        result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &validation_shader);
        if (result != VK_SUCCESS) {
            InternalError(device, loc, "Unable to create shader module. Aborting GPU-AV");
            return nullptr;
        }

        // Create pipeline
        VkPipelineShaderStageCreateInfo pipeline_stage_ci = vku::InitStructHelper();
        pipeline_stage_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipeline_stage_ci.module = validation_shader;
        pipeline_stage_ci.pName = "main";

        VkComputePipelineCreateInfo pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stage = pipeline_stage_ci;
        pipeline_ci.layout = shared_resources->pipeline_layout;

        result = DispatchCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &shared_resources->pipeline);

        DispatchDestroyShaderModule(device, validation_shader, nullptr);

        if (result != VK_SUCCESS) {
            InternalError(device, loc, "Failed to create compute pipeline for pre dispatch validation.");
            return nullptr;
        }
    }

    const auto elt =
        shared_validation_resources_map.insert({typeid(PreDispatchResources::SharedResources), std::move(shared_resources)});

    assert(elt.second);

    return reinterpret_cast<PreDispatchResources::SharedResources *>(elt.first->second.get());
}

void PreDispatchResources::SharedResources::Destroy(Validator &validator) {
    if (ds_layout != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(validator.device, ds_layout, nullptr);
        ds_layout = VK_NULL_HANDLE;
    }
    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(validator.device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }
    if (pipeline != VK_NULL_HANDLE) {
        DispatchDestroyPipeline(validator.device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    if (shader_object != VK_NULL_HANDLE) {
        DispatchDestroyShaderEXT(validator.device, shader_object, nullptr);
        shader_object = VK_NULL_HANDLE;
    }
}

void PreDispatchResources::Destroy(Validator &validator) {
    if (indirect_buffer_desc_set != VK_NULL_HANDLE) {
        validator.desc_set_manager_->PutBackDescriptorSet(desc_pool, indirect_buffer_desc_set);
        indirect_buffer_desc_set = VK_NULL_HANDLE;
        desc_pool = VK_NULL_HANDLE;
    }

    CommandResources::Destroy(validator);
}

}  // namespace gpuav
