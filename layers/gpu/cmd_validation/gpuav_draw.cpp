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
#include "gpu/error_message/gpuav_vuids.h"
#include "gpu/resources/gpuav_subclasses.h"
#include "state_tracker/render_pass_state.h"
#include "gpu/shaders/gpu_error_header.h"
#include "gpu/shaders/gpu_shaders_constants.h"
#include "generated/cmd_validation_draw_vert.h"

// See gpu/shaders/cmd_validation/draw.vert
constexpr uint32_t kPushConstantDWords = 11u;

namespace gpuav {

struct SharedDrawValidationResources final {
    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkShaderEXT shader_object = VK_NULL_HANDLE;
    vvl::concurrent_unordered_map<VkRenderPass, VkPipeline> renderpass_to_pipeline;
    VkDevice device = VK_NULL_HANDLE;

    SharedDrawValidationResources(Validator &gpuav, VkDescriptorSetLayout error_output_desc_set_layout, bool use_shader_objects,
                                  const Location &loc)
        : device(gpuav.device) {
        VkResult result = VK_SUCCESS;

        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},  // count buffer
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},  // draw buffer
        };

        VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
        ds_layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
        ds_layout_ci.pBindings = bindings.data();
        result = DispatchCreateDescriptorSetLayout(device, &ds_layout_ci, nullptr, &ds_layout);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create descriptor set layout for SharedDrawValidationResources.");
            return;
        }

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = kPushConstantDWords * sizeof(uint32_t);

        std::array<VkDescriptorSetLayout, 2> set_layouts = {{error_output_desc_set_layout, ds_layout}};
        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        pipeline_layout_ci.pushConstantRangeCount = 1;
        pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
        pipeline_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
        pipeline_layout_ci.pSetLayouts = set_layouts.data();
        result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &pipeline_layout);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create pipeline layout for SharedDrawValidationResources.");
            return;
        }

        if (use_shader_objects) {
            VkShaderCreateInfoEXT shader_ci = vku::InitStructHelper();
            shader_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shader_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
            shader_ci.codeSize = cmd_validation_draw_vert_size * sizeof(uint32_t);
            shader_ci.pCode = cmd_validation_draw_vert;
            shader_ci.pName = "main";
            shader_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
            shader_ci.pSetLayouts = set_layouts.data();
            shader_ci.pushConstantRangeCount = 1;
            shader_ci.pPushConstantRanges = &push_constant_range;
            result = DispatchCreateShadersEXT(device, 1u, &shader_ci, nullptr, &shader_object);
            if (result != VK_SUCCESS) {
                gpuav.InternalError(device, loc, "Unable to create shader object.");
                return;
            }
        } else {
            VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
            shader_module_ci.codeSize = cmd_validation_draw_vert_size * sizeof(uint32_t);
            shader_module_ci.pCode = cmd_validation_draw_vert;
            result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &shader_module);
            if (result != VK_SUCCESS) {
                gpuav.InternalError(device, loc, "Unable to create shader module.");
                return;
            }
        }
    }

    ~SharedDrawValidationResources() {
        if (shader_module != VK_NULL_HANDLE) {
            DispatchDestroyShaderModule(device, shader_module, nullptr);
            shader_module = VK_NULL_HANDLE;
        }
        if (ds_layout != VK_NULL_HANDLE) {
            DispatchDestroyDescriptorSetLayout(device, ds_layout, nullptr);
            ds_layout = VK_NULL_HANDLE;
        }
        if (pipeline_layout != VK_NULL_HANDLE) {
            DispatchDestroyPipelineLayout(device, pipeline_layout, nullptr);
            pipeline_layout = VK_NULL_HANDLE;
        }
        auto to_destroy = renderpass_to_pipeline.snapshot();
        for (auto &entry : to_destroy) {
            DispatchDestroyPipeline(device, entry.second, nullptr);
            renderpass_to_pipeline.erase(entry.first);
        }
        if (shader_object != VK_NULL_HANDLE) {
            DispatchDestroyShaderEXT(device, shader_object, nullptr);
            shader_object = VK_NULL_HANDLE;
        }
    }

    bool IsValid() const { return shader_module != VK_NULL_HANDLE || shader_object != VK_NULL_HANDLE; }
};

// This function will add the returned VkPipeline handle to another object incharge of destroying it. Caller does NOT have to
// destroy it
static VkPipeline GetDrawValidationPipeline(Validator &gpuav, SharedDrawValidationResources &shared_draw_resources,
                                            VkRenderPass render_pass, const Location &loc) {
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

    VkResult result = DispatchCreateGraphicsPipelines(gpuav.device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &validation_pipeline);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Unable to create graphics pipeline.");
        return VK_NULL_HANDLE;
    }

    shared_draw_resources.renderpass_to_pipeline.insert(render_pass, validation_pipeline);
    return validation_pipeline;
}

void DestroyRenderPassMappedResources(Validator &gpuav, VkRenderPass render_pass) {
    auto *shared_draw_resources = gpuav.shared_resources_manager.TryGet<SharedDrawValidationResources>();

    if (!shared_draw_resources || !shared_draw_resources->IsValid()) {
        return;
    }

    auto pipeline = shared_draw_resources->renderpass_to_pipeline.pop(render_pass);
    if (pipeline != shared_draw_resources->renderpass_to_pipeline.end()) {
        DispatchDestroyPipeline(gpuav.device, pipeline->second, nullptr);
    }
}

void InsertIndirectDrawValidation(Validator &gpuav, const Location &loc, CommandBuffer &cb_state, VkBuffer indirect_buffer,
                                  VkDeviceSize indirect_offset, uint32_t draw_count, VkBuffer count_buffer,
                                  VkDeviceSize count_buffer_offset, uint32_t stride) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;
    const bool use_shader_objects = pipeline_state == nullptr;

    auto &shared_draw_resources = gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(
        gpuav, cb_state.GetValidationCmdCommonDescriptorSetLayout(), use_shader_objects, loc);

    assert(shared_draw_resources.IsValid());
    if (!shared_draw_resources.IsValid()) {
        return;
    }

    VkPipeline validation_pipeline = VK_NULL_HANDLE;
    if (!use_shader_objects) {
        validation_pipeline =
            GetDrawValidationPipeline(gpuav, shared_draw_resources, cb_state.activeRenderPass.get()->VkHandle(), loc);
        if (validation_pipeline == VK_NULL_HANDLE) {
            gpuav.InternalError(cb_state.VkHandle(), loc, "Could not find or create a pipeline.");
            return;
        }
    }

    const VkDescriptorSet draw_validation_desc_set =
        cb_state.gpu_resources_manager.GetManagedDescriptorSet(shared_draw_resources.ds_layout);
    if (draw_validation_desc_set == VK_NULL_HANDLE) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Unable to allocate descriptor set.");
        return;
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
        desc_write.dstSet = draw_validation_desc_set;
    }
    DispatchUpdateDescriptorSets(gpuav.device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, NULL);

    // Insert a draw that can examine some device memory right before the draw we're validating (Pre Draw Validation)
    //
    // NOTE that this validation does not attempt to abort invalid api calls as most other validation does. A crash
    // or DEVICE_LOST resulting from the invalid call will prevent preceeding validation errors from being reported.

    // Save current graphics pipeline state
    const vvl::Func command = loc.function;
    RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);
    using vvl::Func;
    const bool is_mesh_call =
        (command == Func::vkCmdDrawMeshTasksIndirectCountEXT || command == Func::vkCmdDrawMeshTasksIndirectCountNV ||
         command == Func::vkCmdDrawMeshTasksIndirectEXT || command == Func::vkCmdDrawMeshTasksIndirectNV);

    const bool is_count_call =
        (command == Func::vkCmdDrawIndirectCount || command == Func::vkCmdDrawIndirectCountKHR ||
         command == Func::vkCmdDrawIndexedIndirectCount || command == Func::vkCmdDrawIndexedIndirectCountKHR ||
         command == Func::vkCmdDrawMeshTasksIndirectCountEXT || command == Func::vkCmdDrawMeshTasksIndirectCountNV);

    uint32_t push_constants[kPushConstantDWords] = {};
    VkDeviceSize indirect_buffer_size = 0;
    if (is_count_call) {
        // Validate count buffer
        if (count_buffer_offset > std::numeric_limits<uint32_t>::max()) {
            gpuav.InternalError(cb_state.VkHandle(), loc,
                                "Count buffer offset is larger than can be contained in an unsigned int.");
            return;
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
        auto indirect_buffer_state = gpuav.Get<vvl::Buffer>(indirect_buffer);
        indirect_buffer_size = indirect_buffer_state->create_info.size;
        const uint64_t first_command_bytes = struct_size + indirect_offset;
        uint32_t max_count;
        if (first_command_bytes > indirect_buffer_size) {
            max_count = 0;
        } else {
            max_count = 1 + static_cast<uint32_t>(std::floor(((indirect_buffer_size - first_command_bytes) / stride)));
        }

        assert(gpuav.phys_dev_props.limits.maxDrawIndirectCount > 0);
        push_constants[0] = (is_mesh_call) ? glsl::kPreDrawSelectMeshCountBuffer : glsl::kPreDrawSelectCountBuffer;
        push_constants[1] = gpuav.phys_dev_props.limits.maxDrawIndirectCount;
        push_constants[2] = max_count;
        push_constants[3] = static_cast<uint32_t>((count_buffer_offset / sizeof(uint32_t)));
    } else if ((command == Func::vkCmdDrawIndirect || command == Func::vkCmdDrawIndexedIndirect) &&
               !gpuav.enabled_features.drawIndirectFirstInstance) {
        // Validate buffer for firstInstance check instead of count buffer check
        push_constants[0] = glsl::kPreDrawSelectDrawBuffer;
        push_constants[1] = draw_count;
        if (command == Func::vkCmdDrawIndirect) {
            push_constants[2] =
                static_cast<uint32_t>((indirect_offset + offsetof(struct VkDrawIndirectCommand, firstInstance)) / sizeof(uint32_t));
        } else {
            assert(command == Func::vkCmdDrawIndexedIndirect);
            push_constants[2] = static_cast<uint32_t>(
                (indirect_offset + offsetof(struct VkDrawIndexedIndirectCommand, firstInstance)) / sizeof(uint32_t));
        }
        push_constants[3] = stride / sizeof(uint32_t);
    }

    bool emit_task_error = false;
    if (is_mesh_call && gpuav.phys_dev_props.limits.maxPushConstantsSize >= kPushConstantDWords * sizeof(uint32_t)) {
        if (!is_count_call) {
            // Select was set in count check for count call
            push_constants[0] = glsl::kPreDrawSelectMeshNoCount;
        }
        const VkShaderStageFlags stages = pipeline_state->create_info_shaders;
        push_constants[4] = static_cast<uint32_t>(indirect_offset / sizeof(uint32_t));
        push_constants[5] = is_count_call ? 0 : draw_count;
        push_constants[6] = stride / sizeof(uint32_t);
        if (stages & VK_SHADER_STAGE_TASK_BIT_EXT) {
            emit_task_error = true;
            push_constants[7] = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0];
            push_constants[8] = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1];
            push_constants[9] = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2];
            push_constants[10] = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount;
        } else {
            push_constants[7] = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0];
            push_constants[8] = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1];
            push_constants[9] = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2];
            push_constants[10] = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount;
        }
    }

    // Insert diagnostic draw
    if (use_shader_objects) {
        std::array<VkShaderStageFlagBits, 5> stages{{VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                     VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                     VK_SHADER_STAGE_FRAGMENT_BIT}};
        std::array<VkShaderEXT, 5> shaders{{
            shared_draw_resources.shader_object,
            VK_NULL_HANDLE,
            VK_NULL_HANDLE,
            VK_NULL_HANDLE,
            VK_NULL_HANDLE,
        }};
        DispatchCmdBindShadersEXT(cb_state.VkHandle(), static_cast<uint32_t>(stages.size()), stages.data(), shaders.data());
    } else {
        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, validation_pipeline);
    }
    static_assert(sizeof(push_constants) <= 128, "push_constants buffer size >128, need to consider maxPushConstantsSize.");
    DispatchCmdPushConstants(cb_state.VkHandle(), shared_draw_resources.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                             static_cast<uint32_t>(sizeof(push_constants)), push_constants);
    BindValidationCmdsCommonDescSet(gpuav, cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, shared_draw_resources.pipeline_layout,
                                    cb_state.draw_index, static_cast<uint32_t>(cb_state.per_command_error_loggers.size()));
    DispatchCmdBindDescriptorSets(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, shared_draw_resources.pipeline_layout,
                                  glsl::kDiagPerCmdDescriptorSet, 1, &draw_validation_desc_set, 0, nullptr);
    DispatchCmdDraw(cb_state.VkHandle(), 3, 1, 0, 0);  // TODO: this 3 assumes triangles I think, probably could be 1?

    CommandBuffer::ErrorLoggerFunc error_logger = [loc, indirect_buffer, indirect_offset, stride, indirect_buffer_size,
                                                   emit_task_error](Validator &gpuav, const uint32_t *error_record,
                                                                    const LogObjectList &objlist) {
        bool skip = false;

        using namespace glsl;

        if (error_record[kHeaderErrorGroupOffset] != kErrorGroupGpuPreDraw) {
            assert(false);
            return skip;
        }

        const GpuVuid &vuids = GetGpuVuid(loc.function);

        switch (error_record[kHeaderErrorSubCodeOffset]) {
            case kErrorSubCodePreDrawBufferSize: {
                // Buffer size must be >= (stride * (drawCount - 1) + offset + sizeof(VkDrawIndexedIndirectCommand))
                const uint32_t count = error_record[kPreActionParamOffset_0];
                const uint32_t offset = static_cast<uint32_t>(indirect_offset);  // TODO: why cast to uin32_t? If it is changed,
                                                                                 // think about also doing it in the error message
                const uint32_t draw_size = (stride * (count - 1) + offset + sizeof(VkDrawIndexedIndirectCommand));

                const char *vuid = nullptr;
                if (count == 1) {
                    vuid = vuids.count_exceeds_bufsize_1;
                } else {
                    vuid = vuids.count_exceeds_bufsize;
                }
                skip |= gpuav.LogError(vuid, objlist, loc,
                                       "Indirect draw count of %" PRIu32 " would exceed buffer size %" PRIu64
                                       " of buffer %s "
                                       "stride = %" PRIu32 " offset = %" PRIu32
                                       " (stride * (drawCount - 1) + offset + sizeof(VkDrawIndexedIndirectCommand)) = %" PRIu32 ".",
                                       count, indirect_buffer_size, gpuav.FormatHandle(indirect_buffer).c_str(), stride, offset,
                                       draw_size);
                break;
            }
            case kErrorSubCodePreDrawCountLimit: {
                const uint32_t count = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError(vuids.count_exceeds_device_limit, objlist, loc,
                                       "Indirect draw count of %" PRIu32 " would exceed maxDrawIndirectCount limit of %" PRIu32 ".",
                                       count, gpuav.phys_dev_props.limits.maxDrawIndirectCount);
                break;
            }
            case kErrorSubCodePreDrawFirstInstance: {
                const uint32_t index = error_record[kPreActionParamOffset_0];
                gpuav.LogError(
                    vuids.first_instance_not_zero, objlist, loc,
                    "The drawIndirectFirstInstance feature is not enabled, but the firstInstance member of the %s structure at "
                    "index %" PRIu32 " is not zero.",
                    String(loc.function), index);
                break;
            }
            case kErrorSubCodePreDrawGroupCountX:
            case kErrorSubCodePreDrawGroupCountY:
            case kErrorSubCodePreDrawGroupCountZ: {
                const uint32_t group_count = error_record[kPreActionParamOffset_0];
                const uint32_t draw_number = error_record[kPreActionParamOffset_1];
                const char *count_label;
                uint32_t index;
                uint32_t limit;
                const char *vuid;
                if (error_record[kHeaderErrorSubCodeOffset] == kErrorSubCodePreDrawGroupCountX) {
                    count_label = "groupCountX";
                    index = 0;
                    vuid = emit_task_error ? vuids.task_group_count_exceeds_max_x : vuids.mesh_group_count_exceeds_max_x;
                    limit = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0];
                } else if (error_record[kHeaderErrorSubCodeOffset] == kErrorSubCodePreDrawGroupCountY) {
                    count_label = "groupCountY";
                    index = 1;
                    vuid = emit_task_error ? vuids.task_group_count_exceeds_max_y : vuids.mesh_group_count_exceeds_max_y;
                    limit = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1];
                } else {
                    assert(error_record[kHeaderErrorSubCodeOffset] == kErrorSubCodePreDrawGroupCountZ);
                    count_label = "groupCountZ";
                    index = 2;
                    vuid = emit_task_error ? vuids.task_group_count_exceeds_max_z : vuids.mesh_group_count_exceeds_max_z;
                    limit = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2];
                }
                skip |=
                    gpuav.LogError(vuid, objlist, loc,
                                   "In draw %" PRIu32 ", %s is %" PRIu32
                                   " which is greater than VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupCount[%" PRIu32
                                   "] (%" PRIu32 ").",
                                   draw_number, count_label, group_count, index, limit);
                break;
            }
            case kErrorSubCodePreDrawGroupCountTotal: {
                const uint32_t total_count = error_record[kPreActionParamOffset_0];
                const uint32_t draw_number = error_record[kPreActionParamOffset_1];
                auto vuid = emit_task_error ? vuids.task_group_count_exceeds_max_total : vuids.mesh_group_count_exceeds_max_total;
                skip |= gpuav.LogError(
                    vuid, objlist, loc,
                    "In draw %" PRIu32 ", The product of groupCountX, groupCountY and groupCountZ (%" PRIu32
                    ") is greater than VkPhysicalDeviceMeshShaderPropertiesEXT::maxTaskWorkGroupTotalCount (%" PRIu32 ").",
                    draw_number, total_count, gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount);

                break;
            }
            default:
                break;
        }

        return skip;
    };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}

}  // namespace gpuav
