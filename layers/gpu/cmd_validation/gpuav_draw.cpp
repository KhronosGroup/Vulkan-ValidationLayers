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
#include "state_tracker/render_pass_state.h"
// Generated shaders
#include "gpu_shaders/gpu_error_header.h"
#include "generated/cmd_validation_draw_vert.h"

namespace gpuav {

std::unique_ptr<CommandResources> Validator::AllocatePreDrawIndirectValidationResources(
    const Location &loc, VkCommandBuffer cmd_buffer, VkBuffer indirect_buffer, VkDeviceSize indirect_offset, uint32_t draw_count,
    VkBuffer count_buffer, VkDeviceSize count_buffer_offset, uint32_t stride) {
    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(cmd_buffer, loc, "Unrecognized command buffer");
        aborted = true;
        return nullptr;
    }

    if (!gpuav_settings.validate_indirect_draws_buffers) {
        CommandResources cmd_resources = AllocateActionCommandResources(cb_node, VK_PIPELINE_BIND_POINT_GRAPHICS, loc);
        auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
        return cmd_resources_ptr;
    }

    auto draw_resources = std::make_unique<PreDrawResources>();
    {
        const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
        auto const &last_bound = cb_node->lastBound[lv_bind_point];
        const auto *pipeline_state = last_bound.pipeline_state;
        const bool use_shader_objects = pipeline_state == nullptr;

        PreDrawResources::SharedResources *shared_resources =
            GetSharedDrawIndirectValidationResources(cb_node->GetValidationCmdCommonDescriptorSetLayout(), use_shader_objects, loc);
        if (!shared_resources) {
            return nullptr;
        }

        draw_resources->indirect_buffer = indirect_buffer;
        draw_resources->indirect_buffer_offset = indirect_offset;
        draw_resources->indirect_buffer_stride = stride;

        VkPipeline validation_pipeline = VK_NULL_HANDLE;
        if (!use_shader_objects) {
            validation_pipeline = GetDrawValidationPipeline(*shared_resources, cb_node->activeRenderPass.get()->VkHandle(), loc);
            if (validation_pipeline == VK_NULL_HANDLE) {
                ReportSetupProblem(cmd_buffer, loc, "Could not find or create a pipeline. Aborting GPU-AV");
                aborted = true;
                return nullptr;
            }
        }
        VkResult result = VK_SUCCESS;
        result = desc_set_manager->GetDescriptorSet(&draw_resources->desc_pool, shared_resources->ds_layout,
                                                    &draw_resources->buffer_desc_set);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(cmd_buffer, loc, "Unable to allocate descriptor set. Aborting GPU-AV");
            aborted = true;
            return nullptr;
        }

        std::vector<VkDescriptorBufferInfo> buffer_infos;
        buffer_infos.emplace_back(VkDescriptorBufferInfo{indirect_buffer, 0, VK_WHOLE_SIZE});
        if (count_buffer) {
            buffer_infos.emplace_back(VkDescriptorBufferInfo{count_buffer, 0, VK_WHOLE_SIZE});
        }

        std::vector<VkWriteDescriptorSet> desc_writes{};
        for (size_t i = 0; i < buffer_infos.size(); ++i) {
            VkWriteDescriptorSet &desc_write = desc_writes.emplace_back();
            desc_write = vku::InitStructHelper();
            desc_write.dstBinding = uint32_t(i);
            desc_write.descriptorCount = 1;
            desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            desc_write.pBufferInfo = &buffer_infos[i];
            desc_write.dstSet = draw_resources->buffer_desc_set;
        }
        DispatchUpdateDescriptorSets(device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, NULL);

        // Insert a draw that can examine some device memory right before the draw we're validating (Pre Draw Validation)
        //
        // NOTE that this validation does not attempt to abort invalid api calls as most other validation does. A crash
        // or DEVICE_LOST resulting from the invalid call will prevent preceeding validation errors from being reported.

        // Save current graphics pipeline state
        const vvl::Func command = loc.function;
        RestorablePipelineState restorable_state(*cb_node, VK_PIPELINE_BIND_POINT_GRAPHICS);
        const bool is_mesh_call =
            (command == Func::vkCmdDrawMeshTasksIndirectCountEXT || command == Func::vkCmdDrawMeshTasksIndirectCountNV ||
             command == Func::vkCmdDrawMeshTasksIndirectEXT || command == Func::vkCmdDrawMeshTasksIndirectNV);

        const bool is_count_call =
            (command == Func::vkCmdDrawIndirectCount || command == Func::vkCmdDrawIndirectCountKHR ||
             command == Func::vkCmdDrawIndexedIndirectCount || command == Func::vkCmdDrawIndexedIndirectCountKHR ||
             command == Func::vkCmdDrawMeshTasksIndirectCountEXT || command == Func::vkCmdDrawMeshTasksIndirectCountNV);

        uint32_t push_constants[PreDrawResources::push_constant_words] = {};
        if (is_count_call) {
            // Validate count buffer
            if (count_buffer_offset > std::numeric_limits<uint32_t>::max()) {
                ReportSetupProblem(cmd_buffer, loc,
                                   "Count buffer offset is larger than can be contained in an unsigned int. Aborting GPU-AV");
                aborted = true;
                return nullptr;
            }

            // Buffer size must be >= (stride * (drawCount - 1) + offset + sizeof(VkDrawIndirectCommand))
            uint32_t struct_size;
            if (command == Func::vkCmdDrawIndirectCount || command == Func::vkCmdDrawIndirectCountKHR) {
                struct_size = sizeof(VkDrawIndirectCommand);
            } else if (command == Func::vkCmdDrawIndexedIndirectCount || command == Func::vkCmdDrawIndexedIndirectCountKHR) {
                struct_size = sizeof(VkDrawIndexedIndirectCommand);
            } else {
                assert(command == Func::vkCmdDrawMeshTasksIndirectCountEXT || command == Func::vkCmdDrawMeshTasksIndirectCountNV);
                struct_size = sizeof(VkDrawMeshTasksIndirectCommandEXT);
            }
            auto buffer_state = Get<vvl::Buffer>(indirect_buffer);
            uint32_t max_count;
            uint64_t bufsize = buffer_state->create_info.size;
            uint64_t first_command_bytes = struct_size + indirect_offset;
            if (first_command_bytes > bufsize) {
                max_count = 0;
            } else {
                max_count = 1 + static_cast<uint32_t>(std::floor(((bufsize - first_command_bytes) / stride)));
            }
            draw_resources->indirect_buffer_size = bufsize;

            assert(phys_dev_props.limits.maxDrawIndirectCount > 0);
            push_constants[0] = (is_mesh_call) ? glsl::kPreDrawSelectMeshCountBuffer : glsl::kPreDrawSelectCountBuffer;
            push_constants[1] = phys_dev_props.limits.maxDrawIndirectCount;
            push_constants[2] = max_count;
            push_constants[3] = static_cast<uint32_t>((count_buffer_offset / sizeof(uint32_t)));
        } else if ((command == Func::vkCmdDrawIndirect || command == Func::vkCmdDrawIndexedIndirect) &&
                   !enabled_features.drawIndirectFirstInstance) {
            // Validate buffer for firstInstance check instead of count buffer check
            push_constants[0] = glsl::kPreDrawSelectDrawBuffer;
            push_constants[1] = draw_count;
            if (command == Func::vkCmdDrawIndirect) {
                push_constants[2] = static_cast<uint32_t>(
                    (indirect_offset + offsetof(struct VkDrawIndirectCommand, firstInstance)) / sizeof(uint32_t));
            } else {
                assert(command == Func::vkCmdDrawIndexedIndirect);
                push_constants[2] = static_cast<uint32_t>(
                    (indirect_offset + offsetof(struct VkDrawIndexedIndirectCommand, firstInstance)) / sizeof(uint32_t));
            }
            push_constants[3] = stride / sizeof(uint32_t);
        }

        if (is_mesh_call &&
            phys_dev_props.limits.maxPushConstantsSize >= PreDrawResources::push_constant_words * sizeof(uint32_t)) {
            if (!is_count_call) {
                // Select was set in count check for count call
                push_constants[0] = glsl::kPreDrawSelectMeshNoCount;
            }
            const VkShaderStageFlags stages = pipeline_state->create_info_shaders;
            push_constants[4] = static_cast<uint32_t>(indirect_offset / sizeof(uint32_t));
            push_constants[5] = is_count_call ? 0 : draw_count;
            push_constants[6] = stride / sizeof(uint32_t);
            if (stages & VK_SHADER_STAGE_TASK_BIT_EXT) {
                draw_resources->emit_task_error = true;
                push_constants[7] = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0];
                push_constants[8] = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1];
                push_constants[9] = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2];
                push_constants[10] = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount;
            } else {
                push_constants[7] = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0];
                push_constants[8] = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1];
                push_constants[9] = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2];
                push_constants[10] = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount;
            }
        }

        // Insert diagnostic draw
        if (use_shader_objects) {
            VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
            DispatchCmdBindShadersEXT(cmd_buffer, 1u, &stage, &shared_resources->shader_object);
        } else {
            DispatchCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, validation_pipeline);
        }
        static_assert(sizeof(push_constants) <= 128, "push_constants buffer size >128, need to consider maxPushConstantsSize.");
        DispatchCmdPushConstants(cmd_buffer, shared_resources->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                 static_cast<uint32_t>(sizeof(push_constants)), push_constants);
        BindValidationCmdsCommonDescSet(cb_node, VK_PIPELINE_BIND_POINT_GRAPHICS, shared_resources->pipeline_layout,
                                        cb_node->draw_index, static_cast<uint32_t>(cb_node->per_command_resources.size()));
        DispatchCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shared_resources->pipeline_layout,
                                      glsl::kDiagPerCmdDescriptorSet, 1, &draw_resources->buffer_desc_set, 0, nullptr);
        DispatchCmdDraw(cmd_buffer, 3, 1, 0, 0);

        CommandResources cmd_resources = AllocateActionCommandResources(cb_node, VK_PIPELINE_BIND_POINT_GRAPHICS, loc);
        if (aborted) {
            return nullptr;
        }
        CommandResources &base = *draw_resources;
        base = cmd_resources;

        // Restore the previous graphics pipeline state.
        restorable_state.Restore(cmd_buffer);
    }

    return draw_resources;
}

PreDrawResources::SharedResources *Validator::GetSharedDrawIndirectValidationResources(VkDescriptorSetLayout error_output_desc_set,
                                                                                       bool use_shader_objects,
                                                                                       const Location &loc) {
    if (auto shared_resources = shared_validation_resources_map.find(typeid(PreDrawResources::SharedResources));
        shared_resources != shared_validation_resources_map.end()) {
        return reinterpret_cast<PreDrawResources::SharedResources *>(shared_resources->second.get());
    }

    auto shared_resources = std::make_unique<PreDrawResources::SharedResources>();

    VkResult result;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},  // count buffer
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},  // draw buffer
    };

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
    ds_layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
    ds_layout_ci.pBindings = bindings.data();
    result = DispatchCreateDescriptorSetLayout(device, &ds_layout_ci, nullptr, &shared_resources->ds_layout);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Unable to create descriptor set layout. Aborting GPU-AV");
        aborted = true;
        return nullptr;
    }

    VkPushConstantRange push_constant_range = {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = PreDrawResources::push_constant_words * sizeof(uint32_t);

    std::array<VkDescriptorSetLayout, 2> set_layouts = {{error_output_desc_set, shared_resources->ds_layout}};
    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pushConstantRangeCount = 1;
    pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
    pipeline_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
    pipeline_layout_ci.pSetLayouts = set_layouts.data();
    result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &shared_resources->pipeline_layout);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Unable to create pipeline layout. Aborting GPU-AV");
        aborted = true;
        return nullptr;
    }

    if (use_shader_objects) {
        VkShaderCreateInfoEXT shader_ci = vku::InitStructHelper();
        shader_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        shader_ci.codeSize = cmd_validation_draw_vert_size * sizeof(uint32_t);
        shader_ci.pCode = cmd_validation_draw_vert;
        shader_ci.pName = "main";
        shader_ci.setLayoutCount = 1u;
        shader_ci.pSetLayouts = &shared_resources->ds_layout;
        shader_ci.pushConstantRangeCount = 1;
        shader_ci.pPushConstantRanges = &push_constant_range;
        result = DispatchCreateShadersEXT(device, 1u, &shader_ci, nullptr, &shared_resources->shader_object);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, loc, "Unable to create shader object. Aborting GPU-AV");
            aborted = true;
            return nullptr;
        }
    } else {
        VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
        shader_module_ci.codeSize = cmd_validation_draw_vert_size * sizeof(uint32_t);
        shader_module_ci.pCode = cmd_validation_draw_vert;
        result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &shared_resources->shader_module);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, loc, "Unable to create shader module. Aborting GPU-AV");
            aborted = true;
            return nullptr;
        }
    }

    const auto elt =
        shared_validation_resources_map.insert({typeid(PreDrawResources::SharedResources), std::move(shared_resources)});

    assert(elt.second);

    return reinterpret_cast<PreDrawResources::SharedResources *>(elt.first->second.get());
}

PreDrawResources::SharedResources *Validator::GetSharedDrawIndirectValidationResources() {
    if (auto shared_resources = shared_validation_resources_map.find(typeid(PreDrawResources::SharedResources));
        shared_resources != shared_validation_resources_map.end()) {
        return reinterpret_cast<PreDrawResources::SharedResources *>(shared_resources->second.get());
    }
    return nullptr;
}

// This function will add the returned VkPipeline handle to another object incharge of destroying it. Caller does NOT have to
// destroy it
VkPipeline Validator::GetDrawValidationPipeline(PreDrawResources::SharedResources &shared_draw_resources, VkRenderPass render_pass,
                                                const Location &loc) {
    VkPipeline validation_pipeline = VK_NULL_HANDLE;
    // NOTE: for dynamic rendering, render_pass will be VK_NULL_HANDLE but we'll use that as a map
    // key anyways;

    if (auto pipeline_entry = shared_draw_resources.renderpass_to_pipeline.find(render_pass);
        pipeline_entry != shared_draw_resources.renderpass_to_pipeline.end()) {
        validation_pipeline = pipeline_entry->second;
    }
    if (validation_pipeline != VK_NULL_HANDLE) {
        return validation_pipeline;
    }
    VkPipelineShaderStageCreateInfo pipeline_stage_ci = vku::InitStructHelper();
    pipeline_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    pipeline_stage_ci.module = shared_draw_resources.shader_module;
    pipeline_stage_ci.pName = "main";

    VkGraphicsPipelineCreateInfo pipeline_ci = vku::InitStructHelper();
    VkPipelineVertexInputStateCreateInfo vertex_input_state = vku::InitStructHelper();
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = vku::InitStructHelper();
    input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPipelineRasterizationStateCreateInfo rasterization_state = vku::InitStructHelper();
    rasterization_state.rasterizerDiscardEnable = VK_TRUE;
    VkPipelineColorBlendStateCreateInfo color_blend_state = vku::InitStructHelper();

    pipeline_ci.pVertexInputState = &vertex_input_state;
    pipeline_ci.pInputAssemblyState = &input_assembly_state;
    pipeline_ci.pRasterizationState = &rasterization_state;
    pipeline_ci.pColorBlendState = &color_blend_state;
    pipeline_ci.renderPass = render_pass;
    pipeline_ci.layout = shared_draw_resources.pipeline_layout;
    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &pipeline_stage_ci;

    VkResult result = DispatchCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &validation_pipeline);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Unable to create graphics pipeline. Aborting GPU-AV");
        aborted = true;
        return VK_NULL_HANDLE;
    }

    shared_draw_resources.renderpass_to_pipeline.insert(render_pass, validation_pipeline);
    return validation_pipeline;
}

void PreDrawResources::SharedResources::Destroy(Validator &validator) {
    if (shader_module != VK_NULL_HANDLE) {
        DispatchDestroyShaderModule(validator.device, shader_module, nullptr);
        shader_module = VK_NULL_HANDLE;
    }
    if (ds_layout != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(validator.device, ds_layout, nullptr);
        ds_layout = VK_NULL_HANDLE;
    }
    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(validator.device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }
    auto to_destroy = renderpass_to_pipeline.snapshot();
    for (auto &entry : to_destroy) {
        DispatchDestroyPipeline(validator.device, entry.second, nullptr);
        renderpass_to_pipeline.erase(entry.first);
    }
    if (shader_object != VK_NULL_HANDLE) {
        DispatchDestroyShaderEXT(validator.device, shader_object, nullptr);
        shader_object = VK_NULL_HANDLE;
    }
}

void PreDrawResources::Destroy(Validator &validator) {
    if (buffer_desc_set != VK_NULL_HANDLE) {
        validator.desc_set_manager->PutBackDescriptorSet(desc_pool, buffer_desc_set);
        buffer_desc_set = VK_NULL_HANDLE;
        desc_pool = VK_NULL_HANDLE;
    }

    CommandResources::Destroy(validator);
}

}  // namespace gpuav
