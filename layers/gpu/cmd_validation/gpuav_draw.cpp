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

#include "gpu/cmd_validation/gpuav_draw.h"

#include "gpu/core/gpuav.h"
#include "gpu/cmd_validation/gpuav_cmd_validation_common.h"
#include "gpu/error_message/gpuav_vuids.h"
#include "gpu/resources/gpu_resources.h"
#include "gpu/resources/gpuav_subclasses.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/render_pass_state.h"
#include "gpu/shaders/gpu_error_header.h"
#include "gpu/shaders/gpu_shaders_constants.h"
#include "gpu/shaders/cmd_validation/draw_push_data.h"
#include "generated/cmd_validation_draw_mesh_indirect_vert.h"
#include "generated/cmd_validation_first_instance_vert.h"
#include "generated/cmd_validation_count_buffer_vert.h"
#include "generated/cmd_validation_draw_indexed_vert.h"
#include "generated/cmd_validation_draw_indexed_indirect_index_buffer_vert.h"
#include "generated/cmd_validation_draw_indexed_indirect_vertex_buffer_vert.h"

namespace gpuav {
namespace draw {

struct SharedDrawValidationResources {
    VmaAllocator vma_allocator;
    gpu::DeviceMemoryBlock dummy_buffer;  // Used to fill unused buffer bindings in validation pipelines
    bool valid = false;

    SharedDrawValidationResources(Validator &gpuav, const Location &loc) : vma_allocator(gpuav.vma_allocator_) {
        VkBufferCreateInfo dummy_buffer_info = vku::InitStructHelper();
        dummy_buffer_info.size = 64;// whatever
        dummy_buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        dummy_buffer_info.size = dummy_buffer_info.size;
        VkResult result = vmaCreateBuffer(gpuav.vma_allocator_, &dummy_buffer_info, &alloc_info, &dummy_buffer.buffer,
                                          &dummy_buffer.allocation, nullptr);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(gpuav.device, loc, "Unable to allocate device memory for dummy buffer.", true);
            return;
        }

        valid = true;
    }

    ~SharedDrawValidationResources() { dummy_buffer.Destroy(vma_allocator); }
};

// This function will add the returned VkPipeline handle to another object in charge of destroying it. Caller must not destroy it.
static VkPipeline GetDrawValidationPipeline(Validator &gpuav,
                                            vvl::concurrent_unordered_map<VkRenderPass, VkPipeline> &render_passes_to_pipeline,
                                            VkPipelineLayout pipeline_layout, VkShaderModule shader_module,
                                            VkRenderPass render_pass, const Location &loc) {
    // NOTE: for dynamic rendering, render_pass will be VK_NULL_HANDLE but we'll use that as a map
    // key anyways;

    if (auto pipeline_entry = render_passes_to_pipeline.find(render_pass); pipeline_entry != render_passes_to_pipeline.end()) {
        VkPipeline validation_pipeline = pipeline_entry->second;
        assert(validation_pipeline != VK_NULL_HANDLE);
        return validation_pipeline;
    }

    // Create all validation pipeline for the input render_pass
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo pipeline_stage_ci = vku::InitStructHelper();
    pipeline_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    pipeline_stage_ci.module = shader_module;
    pipeline_stage_ci.pName = "main";

    VkGraphicsPipelineCreateInfo pipeline_ci = vku::InitStructHelper();
    VkPipelineVertexInputStateCreateInfo vertex_input_state = vku::InitStructHelper();
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = vku::InitStructHelper();
    input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    VkPipelineRasterizationStateCreateInfo rasterization_state = vku::InitStructHelper();
    rasterization_state.rasterizerDiscardEnable = VK_TRUE;
    VkPipelineColorBlendStateCreateInfo color_blend_state = vku::InitStructHelper();

    pipeline_ci.pVertexInputState = &vertex_input_state;
    pipeline_ci.pInputAssemblyState = &input_assembly_state;
    pipeline_ci.pRasterizationState = &rasterization_state;
    pipeline_ci.pColorBlendState = &color_blend_state;
    pipeline_ci.renderPass = render_pass;
    pipeline_ci.layout = pipeline_layout;
    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &pipeline_stage_ci;

    VkResult result = DispatchCreateGraphicsPipelines(gpuav.device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Unable to create graphics pipeline.");
        return VK_NULL_HANDLE;
    }

    render_passes_to_pipeline.insert(render_pass, pipeline);
    return pipeline;
}

struct ValidationPipeline {
    VkDevice device = VK_NULL_HANDLE;
    VkDescriptorSetLayout specific_desc_set_layout = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkShaderModule shader_module = VK_NULL_HANDLE;
    bool valid = false;

    void Init(Validator &gpuav, const Location &loc, VkDescriptorSetLayout error_output_desc_set_layout,
              const std::vector<VkDescriptorSetLayoutBinding> &specific_bindings, const VkPushConstantRange &push_constant_range,
              size_t spirv_dwords_count, const uint32_t *spirv) {
        device = gpuav.device;
        VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();

        ds_layout_ci.bindingCount = static_cast<uint32_t>(specific_bindings.size());
        ds_layout_ci.pBindings = specific_bindings.data();
        VkResult result = DispatchCreateDescriptorSetLayout(device, &ds_layout_ci, nullptr, &specific_desc_set_layout);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create descriptor set layout for SharedDrawValidationResources.");
            return;
        }

        std::array<VkDescriptorSetLayout, 2> set_layouts = {{error_output_desc_set_layout, specific_desc_set_layout}};
        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        if (push_constant_range.size > 0) {
            pipeline_layout_ci.pushConstantRangeCount = 1;
            pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
        }
        pipeline_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
        pipeline_layout_ci.pSetLayouts = set_layouts.data();
        result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &pipeline_layout);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create pipeline layout for SharedDrawValidationResources.");
            return;
        }

        VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
        shader_module_ci.codeSize = spirv_dwords_count * sizeof(uint32_t);
        shader_module_ci.pCode = spirv;
        result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &shader_module);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create shader module.");
            return;
        }

        valid = true;
    }

    ~ValidationPipeline() {
        if (shader_module != VK_NULL_HANDLE) {
            DispatchDestroyShaderModule(device, shader_module, nullptr);
        }

        if (specific_desc_set_layout != VK_NULL_HANDLE) {
            DispatchDestroyDescriptorSetLayout(device, specific_desc_set_layout, nullptr);
        }

        if (pipeline_layout != VK_NULL_HANDLE) {
            DispatchDestroyPipelineLayout(device, pipeline_layout, nullptr);
        }
    }
};

template <typename ShaderResources>
struct GraphicsValidationPipeline : ValidationPipeline {
    // Why a concurrent map?
    vvl::concurrent_unordered_map<VkRenderPass, VkPipeline> render_passes_to_pipeline;

    GraphicsValidationPipeline(Validator &gpuav, const Location &loc, VkDescriptorSetLayout error_output_desc_set_layout) {
        std::vector<VkDescriptorSetLayoutBinding> specific_bindings = ShaderResources::GetDescriptorSetLayoutBindings();

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = sizeof(ShaderResources::push_constants);  // 0 size is ok here

        Init(gpuav, loc, error_output_desc_set_layout, specific_bindings, push_constant_range,
             ShaderResources::GetSpirvDwordsCount(), ShaderResources::GetSpirv());
    }

    ~GraphicsValidationPipeline() {
        auto to_destroy = render_passes_to_pipeline.snapshot();
        for (auto &entry : to_destroy) {
            DispatchDestroyPipeline(device, entry.second, nullptr);
            // render_passes_to_pipeline.erase(entry.first);
        }
    }

    void BindShaderResources(Validator &gpuav, CommandBuffer &cb_state, ShaderResources &shader_resources) {
        // Error logging resources
        BindErrorLoggingDescriptorSet(gpuav, cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, cb_state.draw_index,
                                      uint32_t(cb_state.per_command_error_loggers.size()));
        // Specific resources
        VkDescriptorSet desc_set = cb_state.gpu_resources_manager.GetManagedDescriptorSet(specific_desc_set_layout);
        std::vector<VkWriteDescriptorSet> desc_writes = shader_resources.GetDescriptorWrites(desc_set);
        DispatchUpdateDescriptorSets(gpuav.device, uint32_t(desc_writes.size()), desc_writes.data(), 0, nullptr);

        DispatchCmdPushConstants(cb_state.VkHandle(), pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                 sizeof(shader_resources.push_constants), &shader_resources.push_constants);

        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout,
                                      shader_resources.desc_set_id, 1, &desc_set, 0, nullptr);
    }
};

struct FirstInstanceValidationShader {
    static size_t GetSpirvDwordsCount() { return cmd_validation_first_instance_vert_size; }
    static const uint32_t *GetSpirv() { return cmd_validation_first_instance_vert; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::FirstInstancePushData push_constants{};
    gpu::BoundStorageBuffer draw_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndirectBuffer};
    gpu::BoundStorageBuffer count_buffer_binding = {gpuav::glsl::kPreDrawBinding_CountBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_IndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,
             nullptr},  // indirect buffer
            {gpuav::glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,
             nullptr},  // count buffer
        };

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) {
        std::vector<VkWriteDescriptorSet> desc_writes(2);

        desc_writes[0] = vku::InitStructHelper();
        desc_writes[0].dstSet = desc_set;
        desc_writes[0].dstBinding = draw_buffer_binding.binding;
        desc_writes[0].dstArrayElement = 0;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[0].pBufferInfo = &draw_buffer_binding.info;

        desc_writes[1] = vku::InitStructHelper();
        desc_writes[1].dstSet = desc_set;
        desc_writes[1].dstBinding = count_buffer_binding.binding;
        desc_writes[1].dstArrayElement = 0;
        desc_writes[1].descriptorCount = 1;
        desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[1].pBufferInfo = &count_buffer_binding.info;

        return desc_writes;
    }
};

void FirstInstance(Validator &gpuav, CommandBuffer &cb_state, const Location &loc, VkBuffer draw_buffer,
                   VkDeviceSize draw_buffer_offset, uint32_t draw_cmds_byte_stride, vvl::Struct draw_indirect_struct_name,
                   uint32_t first_instance_member_pos, uint32_t draw_count, VkBuffer count_buffer, VkDeviceSize count_buffer_offset,
                   const char *vuid) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    if (gpuav.enabled_features.drawIndirectFirstInstance) return;

    RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);

    auto &shared_draw_validation_resources = gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
    if (!shared_draw_validation_resources.valid) return;
    auto &validation_pipeline = gpuav.shared_resources_manager.Get<GraphicsValidationPipeline<FirstInstanceValidationShader>>(
        gpuav, loc, cb_state.GetValidationCmdCommonDescriptorSetLayout());
    if (!validation_pipeline.valid) return;

    auto draw_buffer_state = gpuav.Get<vvl::Buffer>(draw_buffer);
    if (!draw_buffer_state) {
        gpuav.InternalError(LogObjectList(cb_state.VkHandle(), draw_buffer), loc, "buffer must be a valid VkBuffer handle");
        return;
    }

    // Setup shader resources
    // ---
    {
        FirstInstanceValidationShader shader_resources;
        shader_resources.push_constants.draw_cmds_stride_dwords = draw_cmds_byte_stride / sizeof(uint32_t);
        shader_resources.push_constants.first_instance_member_pos = first_instance_member_pos;

        shader_resources.draw_buffer_binding.info = {draw_buffer, draw_buffer_offset, VK_WHOLE_SIZE};
        if (count_buffer) {
            shader_resources.push_constants.flags |= gpuav::glsl::kFirstInstanceFlags_DrawCountFromBuffer;
            shader_resources.count_buffer_binding.info = {count_buffer, count_buffer_offset, sizeof(uint32_t)};
        } else {
            shader_resources.count_buffer_binding.info = {shared_draw_validation_resources.dummy_buffer.buffer, 0, VK_WHOLE_SIZE};
        }

        validation_pipeline.BindShaderResources(gpuav, cb_state, shader_resources);
    }

    // Setup validation pipeline
    // ---
    {
        VkPipeline pipeline_handle =
            GetDrawValidationPipeline(gpuav, validation_pipeline.render_passes_to_pipeline, validation_pipeline.pipeline_layout,
                                      validation_pipeline.shader_module, cb_state.activeRenderPass.get()->VkHandle(), loc);
        if (pipeline_handle == VK_NULL_HANDLE) {
            gpuav.InternalError(cb_state.VkHandle(), loc, "Could not find or create a pipeline.");
            return;
        }

        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_handle);

        uint32_t max_held_draw_cmds = 0;
        if (draw_buffer_state->create_info.size > draw_buffer_offset) {
            max_held_draw_cmds =
                static_cast<uint32_t>((draw_buffer_state->create_info.size - draw_buffer_offset) / draw_cmds_byte_stride);
        }
        draw_count = std::min(draw_count, max_held_draw_cmds);
        DispatchCmdDraw(cb_state.VkHandle(), draw_count, 1, 0, 0);
    }

    // Register error logger
    // ---
    CommandBuffer::ErrorLoggerFunc error_logger =
        [loc, vuid, draw_indirect_struct_name](Validator &gpuav, const uint32_t *error_record, const LogObjectList &objlist) {
            bool skip = false;

            using namespace glsl;

            if (error_record[kHeaderErrorGroupOffset] != kErrorGroupGpuPreDraw) {
                assert(false);
                return skip;
            }

            assert(error_record[kHeaderErrorSubCodeOffset] == kErrorSubCodePreDrawFirstInstance);

            const uint32_t index = error_record[kPreActionParamOffset_0];
            const uint32_t invalid_first_instance = error_record[kPreActionParamOffset_1];
            skip |= gpuav.LogError(
                vuid, objlist, loc,
                "The drawIndirectFirstInstance feature is not enabled, but the firstInstance member of the %s structure at "
                "index %" PRIu32 " is %" PRIu32 ".",
                 vvl::String(draw_indirect_struct_name), index, invalid_first_instance);

            return skip;
        };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}

template <>
void FirstInstance<VkDrawIndirectCommand>(Validator &gpuav, CommandBuffer &cb_state, const Location &loc, VkBuffer draw_buffer,
                                          VkDeviceSize draw_buffer_offset, uint32_t draw_count, VkBuffer count_buffer,
                                          VkDeviceSize count_buffer_offset, const char *vuid) {
    FirstInstance(gpuav, cb_state, loc, draw_buffer, draw_buffer_offset, sizeof(VkDrawIndirectCommand), vvl::Struct::VkDrawIndirectCommand, 3,
                  draw_count, count_buffer, count_buffer_offset, vuid);
}

template <>
void FirstInstance<VkDrawIndexedIndirectCommand>(Validator &gpuav, CommandBuffer &cb_state, const Location &loc,
                                                 VkBuffer draw_buffer, VkDeviceSize draw_buffer_offset, uint32_t draw_count,
                                                 VkBuffer count_buffer, VkDeviceSize count_buffer_offset, const char *vuid) {
    FirstInstance(gpuav, cb_state, loc, draw_buffer, draw_buffer_offset, sizeof(VkDrawIndexedIndirectCommand),
                  vvl::Struct::VkDrawIndexedIndirectCommand, 4, draw_count, count_buffer, count_buffer_offset, vuid);
}

struct CountBufferValidationShader {
    static size_t GetSpirvDwordsCount() { return cmd_validation_count_buffer_vert_size; }
    static const uint32_t *GetSpirv() { return cmd_validation_count_buffer_vert; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::CountBufferPushData push_constants{};
    gpu::BoundStorageBuffer count_buffer_binding = {gpuav::glsl::kPreDrawBinding_CountBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,
             nullptr},  // count buffer
        };

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) {
        std::vector<VkWriteDescriptorSet> desc_writes(1);

        desc_writes[0] = vku::InitStructHelper();
        desc_writes[0].dstSet = desc_set;
        desc_writes[0].dstBinding = count_buffer_binding.binding;
        desc_writes[0].dstArrayElement = 0;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[0].pBufferInfo = &count_buffer_binding.info;

        return desc_writes;
    }
};

void CountBuffer(Validator &gpuav, CommandBuffer &cb_state, const Location &loc, VkBuffer draw_buffer,
                 VkDeviceSize draw_buffer_offset, uint32_t draw_indirect_struct_byte_size, vvl::Struct draw_indirect_struct_name,
                 uint32_t draw_cmds_byte_stride, VkBuffer count_buffer, VkDeviceSize count_buffer_offset,
                 const char *vuid_draw_buffer_size, const char *vuid_max_draw_count) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    if (!gpuav.enabled_features.shaderInt64) {
        return;
    }

    RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);

    auto &shared_draw_validation_resources = gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
    if (!shared_draw_validation_resources.valid) return;
    auto &validation_pipeline = gpuav.shared_resources_manager.Get<GraphicsValidationPipeline<CountBufferValidationShader>>(
        gpuav, loc, cb_state.GetValidationCmdCommonDescriptorSetLayout());
    if (!validation_pipeline.valid) return;

    auto draw_buffer_state = gpuav.Get<vvl::Buffer>(draw_buffer);
    if (!draw_buffer_state) {
        gpuav.InternalError(LogObjectList(cb_state.VkHandle(), draw_buffer), loc, "buffer must be a valid VkBuffer handle");
        return;
    }

    // Setup shader resources
    // ---
    {
        CountBufferValidationShader shader_resources;
        shader_resources.push_constants.draw_cmds_byte_stride = draw_cmds_byte_stride;
        shader_resources.push_constants.draw_buffer_offset = draw_buffer_offset;
        shader_resources.push_constants.draw_buffer_size = draw_buffer_state->create_info.size;
        shader_resources.push_constants.draw_cmd_byte_size = draw_indirect_struct_byte_size;
        shader_resources.push_constants.device_limit_max_draw_indirect_count = gpuav.phys_dev_props.limits.maxDrawIndirectCount;

        shader_resources.count_buffer_binding.info = {count_buffer, count_buffer_offset, sizeof(uint32_t)};

        validation_pipeline.BindShaderResources(gpuav, cb_state, shader_resources);
    }

    // Setup validation pipeline
    // ---
    {
        VkPipeline pipeline_handle =
            GetDrawValidationPipeline(gpuav, validation_pipeline.render_passes_to_pipeline, validation_pipeline.pipeline_layout,
                                      validation_pipeline.shader_module, cb_state.activeRenderPass.get()->VkHandle(), loc);
        if (pipeline_handle == VK_NULL_HANDLE) {
            gpuav.InternalError(cb_state.VkHandle(), loc, "Could not find or create a pipeline.");
            return;
        }

        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_handle);

        DispatchCmdDraw(cb_state.VkHandle(), 1, 1, 0, 0);
    }

    // Register error logger
    // ---
    CommandBuffer::ErrorLoggerFunc error_logger = [loc, draw_buffer, draw_buffer_size = draw_buffer_state->create_info.size,
                                                   draw_buffer_offset, draw_indirect_struct_byte_size, draw_cmds_byte_stride,
                                                   draw_indirect_struct_name, vuid_draw_buffer_size,
                                                   vuid_max_draw_count](Validator &gpuav, const uint32_t *error_record,
                                                                        const LogObjectList &objlist) {
        bool skip = false;

        using namespace glsl;

        switch (error_record[kHeaderErrorSubCodeOffset]) {
            case kErrorSubCodePreDraw_DrawBufferSize: {
                const uint32_t count = error_record[kPreActionParamOffset_0];

                const VkDeviceSize draw_size =
                    (draw_cmds_byte_stride * (count - 1) + draw_buffer_offset + draw_indirect_struct_byte_size);

                skip |= gpuav.LogError(vuid_draw_buffer_size, objlist, loc,
                                       "Indirect draw count of %" PRIu32 " would exceed size (%" PRIu64
                                       ") of buffer (%s). "
                                       "stride = %" PRIu32 " offset = %" PRIu64
                                       " (stride * (drawCount - 1) + offset + sizeof(%s)) = %" PRIu64 ".",
                                       count, draw_buffer_size, gpuav.FormatHandle(draw_buffer).c_str(), draw_cmds_byte_stride,
                                       draw_buffer_offset, vvl::String(draw_indirect_struct_name), draw_size);
                break;
            }
            case kErrorSubCodePreDraw_DrawCountLimit: {
                const uint32_t count = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError(vuid_max_draw_count, objlist, loc,
                                       "Indirect draw count of %" PRIu32 " would exceed maxDrawIndirectCount limit of %" PRIu32 ".",
                                       count, gpuav.phys_dev_props.limits.maxDrawIndirectCount);
                break;
            }
            default:
                assert(false);
                return skip;
        }

        return skip;
    };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}

struct MeshValidationShader {
    static size_t GetSpirvDwordsCount() { return cmd_validation_draw_mesh_indirect_vert_size; }
    static const uint32_t *GetSpirv() { return cmd_validation_draw_mesh_indirect_vert; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::DrawMeshPushData push_constants{};
    gpu::BoundStorageBuffer draw_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndirectBuffer};
    gpu::BoundStorageBuffer count_buffer_binding = {gpuav::glsl::kPreDrawBinding_CountBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_IndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,
             nullptr},  // indirect buffer
            {gpuav::glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,
             nullptr},  // count buffer
        };

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) {
        std::vector<VkWriteDescriptorSet> desc_writes(2);

        desc_writes[0] = vku::InitStructHelper();
        desc_writes[0].dstSet = desc_set;
        desc_writes[0].dstBinding = draw_buffer_binding.binding;
        desc_writes[0].dstArrayElement = 0;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[0].pBufferInfo = &draw_buffer_binding.info;

        desc_writes[1] = vku::InitStructHelper();
        desc_writes[1].dstSet = desc_set;
        desc_writes[1].dstBinding = count_buffer_binding.binding;
        desc_writes[1].dstArrayElement = 0;
        desc_writes[1].descriptorCount = 1;
        desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[1].pBufferInfo = &count_buffer_binding.info;

        return desc_writes;
    }
};

void DrawMeshIndirect(Validator &gpuav, CommandBuffer &cb_state, const Location &loc, VkBuffer draw_buffer,
                      VkDeviceSize draw_buffer_offset, uint32_t draw_cmds_byte_stride, VkBuffer count_buffer,
                      VkDeviceSize count_buffer_offset, uint32_t draw_count, const DrawMeshIndirectVuids &vuids) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;

    auto &shared_draw_validation_resources = gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
    if (!shared_draw_validation_resources.valid) return;
    auto &validation_pipeline = gpuav.shared_resources_manager.Get<GraphicsValidationPipeline<MeshValidationShader>>(
        gpuav, loc, cb_state.GetValidationCmdCommonDescriptorSetLayout());
    if (!validation_pipeline.valid) return;

    auto draw_buffer_state = gpuav.Get<vvl::Buffer>(draw_buffer);
    if (!draw_buffer_state) {
        gpuav.InternalError(LogObjectList(cb_state.VkHandle(), draw_buffer), loc, "buffer must be a valid VkBuffer handle");
        return;
    }

    const VkShaderStageFlags stages = pipeline_state->create_info_shaders;
    const bool is_task_shader = (stages & VK_SHADER_STAGE_TASK_BIT_EXT) == VK_SHADER_STAGE_TASK_BIT_EXT;

    // Setup shader resources
    // ---
    {
        MeshValidationShader shader_resources;
        shader_resources.push_constants.draw_cmds_stride_dwords = draw_cmds_byte_stride / sizeof(uint32_t);
        if (is_task_shader) {
            shader_resources.push_constants.max_workgroup_count_x =
                gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0];
            shader_resources.push_constants.max_workgroup_count_y =
                gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1];
            shader_resources.push_constants.max_workgroup_count_z =
                gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2];
            shader_resources.push_constants.max_workgroup_total_count =
                gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount;
        } else {
            shader_resources.push_constants.max_workgroup_count_x =
                gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0];
            shader_resources.push_constants.max_workgroup_count_y =
                gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1];
            shader_resources.push_constants.max_workgroup_count_z =
                gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2];
            shader_resources.push_constants.max_workgroup_total_count =
                gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount;
        }

        shader_resources.draw_buffer_binding.info = {draw_buffer, draw_buffer_offset, VK_WHOLE_SIZE};
        if (count_buffer != VK_NULL_HANDLE) {
            shader_resources.push_constants.flags |= glsl::kDrawMeshFlags_DrawCountFromBuffer;
            shader_resources.count_buffer_binding.info = {count_buffer, count_buffer_offset, sizeof(uint32_t)};
        } else {
            shader_resources.count_buffer_binding.info = {shared_draw_validation_resources.dummy_buffer.buffer, 0, VK_WHOLE_SIZE};
        }

        validation_pipeline.BindShaderResources(gpuav, cb_state, shader_resources);
    }

    // Setup validation pipeline
    // ---
    {
        VkPipeline pipeline_handle =
            GetDrawValidationPipeline(gpuav, validation_pipeline.render_passes_to_pipeline, validation_pipeline.pipeline_layout,
                                      validation_pipeline.shader_module, cb_state.activeRenderPass.get()->VkHandle(), loc);
        if (pipeline_handle == VK_NULL_HANDLE) {
            gpuav.InternalError(cb_state.VkHandle(), loc, "Could not find or create a pipeline.");
            return;
        }

        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_handle);

        uint32_t max_held_draw_cmds = 0;
        if (draw_buffer_state->create_info.size > draw_buffer_offset) {
            max_held_draw_cmds =
                static_cast<uint32_t>((draw_buffer_state->create_info.size - draw_buffer_offset) / draw_cmds_byte_stride);
        }
        draw_count = std::min(draw_count, max_held_draw_cmds);
        DispatchCmdDraw(cb_state.VkHandle(), draw_count, 1, 0, 0);
    }

    // Register error logger
    // ---
    CommandBuffer::ErrorLoggerFunc error_logger = [loc, vuids, is_task_shader](Validator &gpuav, const uint32_t *error_record,
                                                                               const LogObjectList &objlist) {
        bool skip = false;

        using namespace glsl;

        const uint32_t draw_i = error_record[kPreActionParamOffset_1];
        const char *group_count_name = is_task_shader ? "maxTaskWorkGroupCount" : "maxMeshWorkGroupCount";
        const char *group_count_total_name = is_task_shader ? "maxTaskWorkGroupTotalCount" : "maxMeshWorkGroupTotalCount";

        switch (error_record[kHeaderErrorSubCodeOffset]) {
            case kErrorSubCodePreDrawGroupCountX: {
                const char *vuid_group_count_exceeds_max =
                    is_task_shader ? vuids.task_group_count_exceeds_max_x : vuids.mesh_group_count_exceeds_max_x;
                const uint32_t group_count_x = error_record[kPreActionParamOffset_0];
                const uint32_t limit = is_task_shader ? gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]
                                                      : gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0];
                skip |= gpuav.LogError(vuid_group_count_exceeds_max, objlist, loc,
                                       "In draw %" PRIu32 ", VkDrawMeshTasksIndirectCommandEXT::groupCountX is %" PRIu32
                                       " which is greater than VkPhysicalDeviceMeshShaderPropertiesEXT::%s[0]"
                                       " (%" PRIu32 ").",
                                       draw_i, group_count_x, group_count_name, limit);
                break;
            }

            case kErrorSubCodePreDrawGroupCountY: {
                const char *vuid_group_count_exceeds_max =
                    is_task_shader ? vuids.task_group_count_exceeds_max_y : vuids.mesh_group_count_exceeds_max_y;
                const uint32_t group_count_y = error_record[kPreActionParamOffset_0];
                const uint32_t limit = is_task_shader ? gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1]
                                                      : gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1];
                skip |= gpuav.LogError(vuid_group_count_exceeds_max, objlist, loc,
                                       "In draw %" PRIu32 ", VkDrawMeshTasksIndirectCommandEXT::groupCountY is %" PRIu32
                                       " which is greater than VkPhysicalDeviceMeshShaderPropertiesEXT::%s[1]"
                                       " (%" PRIu32 ").",
                                       draw_i, group_count_y, group_count_name, limit);
                break;
            }

            case kErrorSubCodePreDrawGroupCountZ: {
                const char *vuid_group_count_exceeds_max =
                    is_task_shader ? vuids.task_group_count_exceeds_max_z : vuids.mesh_group_count_exceeds_max_z;
                const uint32_t group_count_z = error_record[kPreActionParamOffset_0];
                const uint32_t limit = is_task_shader ? gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2]
                                                      : gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2];
                skip |= gpuav.LogError(vuid_group_count_exceeds_max, objlist, loc,
                                       "In draw %" PRIu32 ", VkDrawMeshTasksIndirectCommandEXT::groupCountZ is %" PRIu32
                                       " which is greater than VkPhysicalDeviceMeshShaderPropertiesEXT::%s[2]"
                                       " (%" PRIu32 ").",
                                       draw_i, group_count_z, group_count_name, limit);
                break;
            }

            case kErrorSubCodePreDrawGroupCountTotal: {
                const char *vuid_group_count_exceeds_max =
                    is_task_shader ? vuids.task_group_count_exceeds_max_total : vuids.mesh_group_count_exceeds_max_total;
                const uint32_t group_count_total = error_record[kPreActionParamOffset_0];
                const uint32_t limit = is_task_shader ? gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount
                                                      : gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount;
                skip |= gpuav.LogError(vuid_group_count_exceeds_max, objlist, loc,
                                       "In draw %" PRIu32 ", size of VkDrawMeshTasksIndirectCommandEXT is %" PRIu32
                                       " which is greater than VkPhysicalDeviceMeshShaderPropertiesEXT::%s"
                                       " (%" PRIu32 ").",
                                       draw_i, group_count_total, group_count_total_name, limit);
                break;
            }

            default:
                assert(false);
                return skip;
        }

        return skip;
    };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}

struct IndexBufferValidationShader {
    static size_t GetSpirvDwordsCount() { return cmd_validation_draw_indexed_vert_size; }
    static const uint32_t *GetSpirv() { return cmd_validation_draw_indexed_vert; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::IndexedDrawPushData push_constants{};
    gpu::BoundStorageBuffer index_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndexBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_IndexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) {
        std::vector<VkWriteDescriptorSet> desc_writes(1);

        desc_writes[0] = vku::InitStructHelper();
        desc_writes[0].dstSet = desc_set;
        desc_writes[0].dstBinding = index_buffer_binding.binding;
        desc_writes[0].dstArrayElement = 0;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[0].pBufferInfo = &index_buffer_binding.info;

        return desc_writes;
    }
};

struct SmallestVertexBufferBinding {
    VkDeviceSize min_vertex_attributes_count = std::numeric_limits<VkDeviceSize>::max();
    uint32_t binding = std::numeric_limits<uint32_t>::max();
    vvl::VertexBufferBinding binding_info{};
};
// Computes the smallest vertex attributes count among the set of bound vertex buffers.
// Used to detect out of bounds indices in index buffers.
// If no vertex buffer is bound, min_vertex_attributes_count is std::numeric_limits<uint32_t>::max()
// indicating that no index can be out of bound
static SmallestVertexBufferBinding SmallestVertexAttributesCount(const vvl::CommandBuffer &cb_state) {
    // If there is no bound vertex buffers, cannot have

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;
    const bool use_shader_objects = pipeline_state == nullptr;

    const bool dynamic_vertex_input = use_shader_objects || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT);

    const auto &vertex_binding_descriptions =
        dynamic_vertex_input ? cb_state.dynamic_state_value.vertex_bindings : pipeline_state->vertex_input_state->bindings;

    SmallestVertexBufferBinding smallest_vertex_buffer_binding;

    for (const auto &[binding, vertex_binding_desc] : vertex_binding_descriptions) {
        auto find_vbb = cb_state.current_vertex_buffer_binding_info.find(binding);
        if (find_vbb == cb_state.current_vertex_buffer_binding_info.cend()) {
            // This is a validation error
            continue;
        }

        const vvl::VertexBufferBinding &vbb = find_vbb->second;

        for (const auto &[Location, attrib] : vertex_binding_desc.locations) {
            const VkDeviceSize attribute_size = vkuFormatElementSize(attrib.desc.format);

            const VkDeviceSize stride = vbb.stride;  // Tracked stride should already handle all possible value origin

            if (vbb.size < (vbb.offset + attrib.desc.offset)) {
                // overflow will occur
                continue;
            }
            VkDeviceSize vertex_buffer_remaining_size = vbb.size - vbb.offset - attrib.desc.offset;

            VkDeviceSize vertex_attributes_count = 0;
            vertex_attributes_count = vertex_buffer_remaining_size / stride;
            vertex_buffer_remaining_size -= vertex_attributes_count * stride;

            // maybe room for one more attribute but not full stride - not having stride space does not matter for last element
            if (vertex_buffer_remaining_size >= attribute_size) {
                vertex_attributes_count += 1;
            }

            smallest_vertex_buffer_binding.min_vertex_attributes_count =
                std::min(smallest_vertex_buffer_binding.min_vertex_attributes_count, vertex_attributes_count);
            if (smallest_vertex_buffer_binding.min_vertex_attributes_count == vertex_attributes_count) {
                smallest_vertex_buffer_binding.binding = binding;
                smallest_vertex_buffer_binding.binding_info = vbb;
            }
        }
    }
    return smallest_vertex_buffer_binding;
}

void DrawIndexed(Validator &gpuav, CommandBuffer &cb_state, const Location &loc, uint32_t index_count, uint32_t first_index,
                 uint32_t vertex_offset, const char *vuid_oob_vertex) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    if (gpuav.enabled_features.robustBufferAccess) {
        return;
    }

    if (!cb_state.index_buffer_binding.buffer) {
        return;
    }

    const SmallestVertexBufferBinding smallest_vertex_buffer_binding = SmallestVertexAttributesCount(cb_state);
    if (smallest_vertex_buffer_binding.min_vertex_attributes_count == std::numeric_limits<VkDeviceSize>::max()) {
        // cannot overrun index buffer, skip validation
        return;
    }

    RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);

    auto &shared_draw_validation_resources = gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
    if (!shared_draw_validation_resources.valid) return;
    auto &validation_pipeline = gpuav.shared_resources_manager.Get<GraphicsValidationPipeline<IndexBufferValidationShader>>(
        gpuav, loc, cb_state.GetValidationCmdCommonDescriptorSetLayout());
    if (!validation_pipeline.valid) return;

    const uint32_t index_bits_size = GetIndexBitsSize(cb_state.index_buffer_binding.index_type);
    const uint32_t max_indices_in_buffer = static_cast<uint32_t>(cb_state.index_buffer_binding.size / (index_bits_size / 8u));

    {
        IndexBufferValidationShader shader_resources;
        shader_resources.push_constants.smallest_vertex_attributes_count =
            static_cast<uint32_t>(smallest_vertex_buffer_binding.min_vertex_attributes_count);
        shader_resources.push_constants.index_width = index_bits_size;
        shader_resources.push_constants.vertex_offset = vertex_offset;

        shader_resources.index_buffer_binding.info = {cb_state.index_buffer_binding.buffer, cb_state.index_buffer_binding.offset,
                                                      cb_state.index_buffer_binding.size};

        validation_pipeline.BindShaderResources(gpuav, cb_state, shader_resources);
    }

    {
        VkPipeline pipeline_handle =
            GetDrawValidationPipeline(gpuav, validation_pipeline.render_passes_to_pipeline, validation_pipeline.pipeline_layout,
                                      validation_pipeline.shader_module, cb_state.activeRenderPass.get()->VkHandle(), loc);
        if (pipeline_handle == VK_NULL_HANDLE) {
            gpuav.InternalError(cb_state.VkHandle(), loc, "Could not find or create a pipeline.");
            return;
        }

        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_handle);

        // Do not overrun index buffer
        const uint32_t draw_count = std::min(index_count, max_indices_in_buffer);
        DispatchCmdDraw(cb_state.VkHandle(), draw_count, 1, first_index, 0);
    }

    CommandBuffer::ErrorLoggerFunc error_logger =
        [loc, vuid_oob_vertex, smallest_vertex_buffer_binding, index_buffer_binding = cb_state.index_buffer_binding,
         max_indices_in_buffer](Validator &gpuav, const uint32_t *error_record, const LogObjectList &objlist) {
            bool skip = false;

            using namespace glsl;

            switch (error_record[kHeaderErrorSubCodeOffset]) {
                case kErrorSubCode_OobVertexBuffer: {
                    const uint32_t index_buffer_offset = error_record[kPreActionParamOffset_0];
                    const int32_t vertex_offset = static_cast<int32_t>(error_record[kPreActionParamOffset_1]);
                    const uint32_t vertex_index = error_record[kPreActionParamOffset_2];
                    const uint32_t index_buffer_value = static_cast<uint32_t>(int32_t(vertex_index) - vertex_offset);

                    gpuav.LogError(vuid_oob_vertex, objlist, loc,
                                   "Vertex index %" PRIu32
                                   " is not within the smallest bound vertex buffer.\n"
                                   "index_buffer[%" PRIu32 "] (%" PRIu32 ") + vertexOffset (%" PRIi32 ") = Vertex index %" PRIu32
                                   "\n"

                                   "Smallest vertex buffer binding info:\n"
                                   "- Buffer: %s\n"
                                   "- Binding: %" PRIu32
                                   "\n"
                                   "- Binding offset: %" PRIu64
                                   "\n"
                                   "- Binding size: %" PRIu64
                                   " bytes\n"

                                   "Index buffer binding info:\n"
                                   "- Buffer: %s\n"
                                   "- Index type: %s\n"
                                   "- Binding offset: %" PRIu64
                                   "\n"
                                   "- Binding size: %" PRIu64 " bytes (or %" PRIu32 " %s)\n",
                                   // OOB vertex index info
                                   vertex_index, index_buffer_offset, index_buffer_value, vertex_offset, vertex_index,

                                   // Vertex buffer binding info
                                   gpuav.FormatHandle(smallest_vertex_buffer_binding.binding_info.buffer).c_str(),
                                   smallest_vertex_buffer_binding.binding, smallest_vertex_buffer_binding.binding_info.offset,
                                   smallest_vertex_buffer_binding.binding_info.size,

                                   // Index buffer binding info
                                   gpuav.FormatHandle(index_buffer_binding.buffer).c_str(),
                                   string_VkIndexType(index_buffer_binding.index_type), index_buffer_binding.offset,
                                   index_buffer_binding.size, max_indices_in_buffer,
                                   string_VkIndexType(index_buffer_binding.index_type));
                    break;
                }

                default:
                    assert(false);
                    return skip;
            }

            return skip;
        };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}

struct DrawIndexedIndirectIndexBufferShader {
    static size_t GetSpirvDwordsCount() { return cmd_validation_draw_indexed_indirect_index_buffer_vert_size; }
    static const uint32_t *GetSpirv() { return cmd_validation_draw_indexed_indirect_index_buffer_vert; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::DrawIndexedIndirectIndexBufferPushData push_constants{};
    gpu::BoundStorageBuffer draw_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndirectBuffer};
    gpu::BoundStorageBuffer count_buffer_binding = {gpuav::glsl::kPreDrawBinding_CountBuffer};
    gpu::BoundStorageBuffer index_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndexBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_IndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,
             nullptr},
            {gpuav::glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
            {gpuav::glsl::kPreDrawBinding_IndexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) {
        std::vector<VkWriteDescriptorSet> desc_writes(3);

        desc_writes[0] = vku::InitStructHelper();
        desc_writes[0].dstSet = desc_set;
        desc_writes[0].dstBinding = draw_buffer_binding.binding;
        desc_writes[0].dstArrayElement = 0;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[0].pBufferInfo = &draw_buffer_binding.info;

        desc_writes[1] = vku::InitStructHelper();
        desc_writes[1].dstSet = desc_set;
        desc_writes[1].dstBinding = count_buffer_binding.binding;
        desc_writes[1].dstArrayElement = 0;
        desc_writes[1].descriptorCount = 1;
        desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[1].pBufferInfo = &count_buffer_binding.info;

        desc_writes[2] = vku::InitStructHelper();
        desc_writes[2].dstSet = desc_set;
        desc_writes[2].dstBinding = index_buffer_binding.binding;
        desc_writes[2].dstArrayElement = 0;
        desc_writes[2].descriptorCount = 1;
        desc_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[2].pBufferInfo = &index_buffer_binding.info;

        return desc_writes;
    }
};

void DrawIndexedIndirectIndexBuffer(Validator &gpuav, CommandBuffer &cb_state, const Location &loc, VkBuffer draw_buffer,
                                    VkDeviceSize draw_buffer_offset, uint32_t draw_cmds_byte_stride, uint32_t draw_count,
                                    VkBuffer count_buffer, VkDeviceSize count_buffer_offset, const char *vuid_oob_index) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    if (gpuav.enabled_features.robustBufferAccess2) {
        return;
    }

    if (!cb_state.index_buffer_binding.buffer) {
        return;
    }

    RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);

    auto &shared_draw_validation_resources = gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
    if (!shared_draw_validation_resources.valid) return;
    auto &validation_pipeline =
        gpuav.shared_resources_manager.Get<GraphicsValidationPipeline<DrawIndexedIndirectIndexBufferShader>>(
            gpuav, loc, cb_state.GetValidationCmdCommonDescriptorSetLayout());
    if (!validation_pipeline.valid) return;

    const uint32_t index_bits_size = GetIndexBitsSize(cb_state.index_buffer_binding.index_type);
    const uint32_t max_indices_in_buffer = static_cast<uint32_t>(cb_state.index_buffer_binding.size / (index_bits_size / 8u));
    {
        DrawIndexedIndirectIndexBufferShader shader_resources;
        if (count_buffer != VK_NULL_HANDLE) {
            shader_resources.push_constants.flags |= glsl::kIndexedIndirectDrawFlags_DrawCountFromBuffer;
            shader_resources.count_buffer_binding.info = {count_buffer, count_buffer_offset, sizeof(uint32_t)};
        } else {
            shader_resources.count_buffer_binding.info = {shared_draw_validation_resources.dummy_buffer.buffer, 0, VK_WHOLE_SIZE};
        }

        shader_resources.push_constants.draw_cmds_stride_dwords = draw_cmds_byte_stride / sizeof(uint32_t);
        shader_resources.push_constants.bound_index_buffer_indices_count = max_indices_in_buffer;
        shader_resources.push_constants.cpu_draw_count = draw_count;

        shader_resources.draw_buffer_binding.info = {draw_buffer, draw_buffer_offset, VK_WHOLE_SIZE};
        shader_resources.index_buffer_binding.info = {cb_state.index_buffer_binding.buffer, cb_state.index_buffer_binding.offset,
                                                      cb_state.index_buffer_binding.size};

        validation_pipeline.BindShaderResources(gpuav, cb_state, shader_resources);
    }

    {
        VkPipeline pipeline_handle =
            GetDrawValidationPipeline(gpuav, validation_pipeline.render_passes_to_pipeline, validation_pipeline.pipeline_layout,
                                      validation_pipeline.shader_module, cb_state.activeRenderPass.get()->VkHandle(), loc);
        if (pipeline_handle == VK_NULL_HANDLE) {
            gpuav.InternalError(cb_state.VkHandle(), loc, "Could not find or create a pipeline.");
            return;
        }

        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_handle);

        // One draw will check all VkDrawIndexedIndirectCommand
        DispatchCmdDraw(cb_state.VkHandle(), 1, 1, 0, 0);
    }

    CommandBuffer::ErrorLoggerFunc error_logger = [loc, vuid_oob_index, draw_buffer, draw_buffer_offset, draw_cmds_byte_stride,
                                                   index_buffer_binding = cb_state.index_buffer_binding,
                                                   max_indices_in_buffer](Validator &gpuav, const uint32_t *error_record,
                                                                          const LogObjectList &objlist) {
        bool skip = false;

        using namespace glsl;

        switch (error_record[kHeaderErrorSubCodeOffset]) {
            case kErrorSubCode_OobIndexBuffer: {
                const uint32_t draw_i = error_record[kPreActionParamOffset_0];
                const uint32_t first_index = error_record[kPreActionParamOffset_1];
                const uint32_t index_count = error_record[kPreActionParamOffset_2];
                const uint32_t highest_accessed_index = first_index + index_count;

                gpuav.LogError(
                    vuid_oob_index, objlist, loc,
                    "Index %" PRIu32 " is not within the bound index buffer. Computed from VkDrawIndexedIndirectCommand[%" PRIu32
                    "] (.firstIndex = %" PRIu32 ", .indexCount = %" PRIu32
                    "), stored in %s\n"

                    "Index buffer binding info:\n"
                    "- Buffer: %s\n"
                    "- Index type: %s\n"
                    "- Binding offset: %" PRIu64
                    "\n"
                    "- Binding size: %" PRIu64 " bytes (or %" PRIu32
                    " %s)\n"

                    "Supplied buffer parameters in indirect command: offset = %" PRIu64 ", stride = %" PRIu32 " bytes.",
                    // OOB index info
                    highest_accessed_index, draw_i, first_index, index_count, gpuav.FormatHandle(draw_buffer).c_str(),

                    // Index buffer binding info
                    gpuav.FormatHandle(index_buffer_binding.buffer).c_str(), string_VkIndexType(index_buffer_binding.index_type),
                    index_buffer_binding.offset, index_buffer_binding.size, max_indices_in_buffer,
                    string_VkIndexType(index_buffer_binding.index_type),

                    // VkDrawIndexedIndirectCommand info
                    draw_buffer_offset, draw_cmds_byte_stride);
                break;
            }

            default:
                assert(false);
                return skip;
        }

        return skip;
    };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}

struct DrawIndexedIndirectVertexBufferShader {
    static size_t GetSpirvDwordsCount() { return cmd_validation_draw_indexed_indirect_vertex_buffer_vert_size; }
    static const uint32_t *GetSpirv() { return cmd_validation_draw_indexed_indirect_vertex_buffer_vert; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::DrawIndexedIndirectVertexBufferPushData push_constants{};
    gpu::BoundStorageBuffer draw_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndirectBuffer};
    gpu::BoundStorageBuffer count_buffer_binding = {gpuav::glsl::kPreDrawBinding_CountBuffer};
    gpu::BoundStorageBuffer index_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndexBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_IndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,
             nullptr},
            {gpuav::glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
            {gpuav::glsl::kPreDrawBinding_IndexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) {
        std::vector<VkWriteDescriptorSet> desc_writes(3);

        desc_writes[0] = vku::InitStructHelper();
        desc_writes[0].dstSet = desc_set;
        desc_writes[0].dstBinding = draw_buffer_binding.binding;
        desc_writes[0].dstArrayElement = 0;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[0].pBufferInfo = &draw_buffer_binding.info;

        desc_writes[1] = vku::InitStructHelper();
        desc_writes[1].dstSet = desc_set;
        desc_writes[1].dstBinding = count_buffer_binding.binding;
        desc_writes[1].dstArrayElement = 0;
        desc_writes[1].descriptorCount = 1;
        desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[1].pBufferInfo = &count_buffer_binding.info;

        desc_writes[2] = vku::InitStructHelper();
        desc_writes[2].dstSet = desc_set;
        desc_writes[2].dstBinding = index_buffer_binding.binding;
        desc_writes[2].dstArrayElement = 0;
        desc_writes[2].descriptorCount = 1;
        desc_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[2].pBufferInfo = &index_buffer_binding.info;

        return desc_writes;
    }
};

void DrawIndexedIndirectVertexBuffer(Validator &gpuav, CommandBuffer &cb_state, const Location &loc, VkBuffer draw_buffer,
                                     VkDeviceSize draw_buffer_offset, uint32_t draw_cmds_byte_stride, uint32_t draw_count,
                                     VkBuffer count_buffer, VkDeviceSize count_buffer_offset, const char *vuid_oob_vertex) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    if (gpuav.enabled_features.robustBufferAccess) {
        return;
    }

    if (!cb_state.index_buffer_binding.buffer) {
        return;
    }

    RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);

    auto &shared_draw_validation_resources = gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
    if (!shared_draw_validation_resources.valid) return;
    auto &validation_pipeline =
        gpuav.shared_resources_manager.Get<GraphicsValidationPipeline<DrawIndexedIndirectVertexBufferShader>>(
            gpuav, loc, cb_state.GetValidationCmdCommonDescriptorSetLayout());
    if (!validation_pipeline.valid) return;

    const uint32_t index_bits_size = GetIndexBitsSize(cb_state.index_buffer_binding.index_type);
    const uint32_t max_indices_in_buffer = static_cast<uint32_t>(cb_state.index_buffer_binding.size / (index_bits_size / 8u));
    const SmallestVertexBufferBinding smallest_vertex_buffer_binding = SmallestVertexAttributesCount(cb_state);

    {
        DrawIndexedIndirectVertexBufferShader shader_resources;
        if (count_buffer != VK_NULL_HANDLE) {
            shader_resources.push_constants.flags |= glsl::kIndexedIndirectDrawFlags_DrawCountFromBuffer;
            shader_resources.count_buffer_binding.info = {count_buffer, count_buffer_offset, sizeof(uint32_t)};
        } else {
            shader_resources.count_buffer_binding.info = {shared_draw_validation_resources.dummy_buffer.buffer, 0, VK_WHOLE_SIZE};
        }

        shader_resources.push_constants.index_width = index_bits_size;
        shader_resources.push_constants.draw_cmds_stride_dwords = draw_cmds_byte_stride / sizeof(uint32_t);
        shader_resources.push_constants.bound_index_buffer_indices_count = max_indices_in_buffer;
        shader_resources.push_constants.cpu_draw_count = draw_count;

        shader_resources.push_constants.smallest_vertex_attributes_count =
            static_cast<uint32_t>(smallest_vertex_buffer_binding.min_vertex_attributes_count);

        shader_resources.draw_buffer_binding.info = {draw_buffer, draw_buffer_offset, VK_WHOLE_SIZE};
        shader_resources.index_buffer_binding.info = {cb_state.index_buffer_binding.buffer, cb_state.index_buffer_binding.offset,
                                                      cb_state.index_buffer_binding.size};

        validation_pipeline.BindShaderResources(gpuav, cb_state, shader_resources);
    }

    {
        VkPipeline pipeline_handle =
            GetDrawValidationPipeline(gpuav, validation_pipeline.render_passes_to_pipeline, validation_pipeline.pipeline_layout,
                                      validation_pipeline.shader_module, cb_state.activeRenderPass.get()->VkHandle(), loc);
        if (pipeline_handle == VK_NULL_HANDLE) {
            gpuav.InternalError(cb_state.VkHandle(), loc, "Could not find or create a pipeline.");
            return;
        }

        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_handle);

        // Dispatch as many draws as there are indices in index buffer.
        // Each shader invocation will inspect one index, for all draw commands.
        // Shader is in charge of performing validation only for indices within the supplied ranges.

        DispatchCmdDraw(cb_state.VkHandle(), max_indices_in_buffer, 1, 0, 0);
    }

    CommandBuffer::ErrorLoggerFunc error_logger =
        [loc, vuid_oob_vertex, draw_buffer, draw_buffer_offset, draw_cmds_byte_stride, smallest_vertex_buffer_binding,
         index_buffer_binding = cb_state.index_buffer_binding,
         max_indices_in_buffer](Validator &gpuav, const uint32_t *error_record, const LogObjectList &objlist) {
            bool skip = false;

            using namespace glsl;

            switch (error_record[kHeaderErrorSubCodeOffset]) {
                case kErrorSubCode_OobVertexBuffer: {
                    const uint32_t draw_i = error_record[kPreActionParamOffset_0];
                    const uint32_t index_buffer_offset = error_record[kPreActionParamOffset_1];
                    const int32_t vertex_offset = static_cast<int32_t>(error_record[kPreActionParamOffset_2]);
                    const uint32_t vertex_index = error_record[kPreActionParamOffset_3];
                    const uint32_t index_buffer_value = static_cast<uint32_t>(int32_t(vertex_index) - vertex_offset);

                    gpuav.LogError(
                        vuid_oob_vertex, objlist, loc,
                        "Vertex index %" PRIu32
                        " is not within the smallest bound vertex buffer. Computed from VkDrawIndexedIndirectCommand[%" PRIu32
                        "], stored in %s\n"
                        "index_buffer[%" PRIu32 "] (%" PRIu32 ") + VkDrawIndexedIndirectCommand[%" PRIu32 "].vertexOffset (%" PRIi32
                        ") = Vertex index %" PRIu32
                        "\n"

                        "Smallest vertex buffer binding info:\n"
                        "- Buffer: %s\n"
                        "- Binding: %" PRIu32
                        "\n"
                        "- Binding offset: %" PRIu64
                        "\n"
                        "- Binding size: %" PRIu64
                        " bytes\n"

                        "Index buffer binding info:\n"
                        "- Buffer: %s\n"
                        "- Index type: %s\n"
                        "- Binding offset: %" PRIu64
                        "\n"
                        "- Binding size: %" PRIu64 " bytes (or %" PRIu32
                        " %s)\n"

                        "Supplied buffer parameters in indirect command: offset = %" PRIu64 ", stride = %" PRIu32 " bytes.",
                        // OOB vertex index info
                        vertex_index, draw_i, gpuav.FormatHandle(draw_buffer).c_str(), index_buffer_offset, index_buffer_value,
                        draw_i, vertex_offset, vertex_index,

                        // Vertex buffer binding info
                        gpuav.FormatHandle(smallest_vertex_buffer_binding.binding_info.buffer).c_str(),
                        smallest_vertex_buffer_binding.binding, smallest_vertex_buffer_binding.binding_info.offset,
                        smallest_vertex_buffer_binding.binding_info.size,

                        // Index buffer binding info
                        gpuav.FormatHandle(index_buffer_binding.buffer).c_str(),
                        string_VkIndexType(index_buffer_binding.index_type), index_buffer_binding.offset, index_buffer_binding.size,
                        max_indices_in_buffer, string_VkIndexType(index_buffer_binding.index_type),

                        // VkDrawIndexedIndirectCommand info
                        draw_buffer_offset, draw_cmds_byte_stride);
                    break;
                }

                default:
                    assert(false);
                    return skip;
            }

            return skip;
        };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}

void DestroyRenderPassMappedResources(Validator &gpuav, VkRenderPass render_pass) {
    // #ARNO_TODO: Come up with a lambda registration system
    auto *first_instance_validation_pipeline =
        gpuav.shared_resources_manager.TryGet<GraphicsValidationPipeline<FirstInstanceValidationShader>>();

    if (first_instance_validation_pipeline && first_instance_validation_pipeline->valid) {
        auto pipeline = first_instance_validation_pipeline->render_passes_to_pipeline.pop(render_pass);
        if (pipeline != first_instance_validation_pipeline->render_passes_to_pipeline.end()) {
            DispatchDestroyPipeline(gpuav.device, pipeline->second, nullptr);
        }
    }

    auto *count_buffer_validation_pipeline =
        gpuav.shared_resources_manager.TryGet<GraphicsValidationPipeline<CountBufferValidationShader>>();

    if (count_buffer_validation_pipeline && count_buffer_validation_pipeline->valid) {
        auto pipeline = count_buffer_validation_pipeline->render_passes_to_pipeline.pop(render_pass);
        if (pipeline != count_buffer_validation_pipeline->render_passes_to_pipeline.end()) {
            DispatchDestroyPipeline(gpuav.device, pipeline->second, nullptr);
        }
    }

    auto *draw_mesh_validation_pipeline = gpuav.shared_resources_manager.TryGet<GraphicsValidationPipeline<MeshValidationShader>>();

    if (draw_mesh_validation_pipeline && draw_mesh_validation_pipeline->valid) {
        auto pipeline = draw_mesh_validation_pipeline->render_passes_to_pipeline.pop(render_pass);
        if (pipeline != draw_mesh_validation_pipeline->render_passes_to_pipeline.end()) {
            DispatchDestroyPipeline(gpuav.device, pipeline->second, nullptr);
        }
    }

    auto *indexed_draw_validation_pipeline =
        gpuav.shared_resources_manager.TryGet<GraphicsValidationPipeline<IndexBufferValidationShader>>();

    if (indexed_draw_validation_pipeline && indexed_draw_validation_pipeline->valid) {
        auto pipeline = indexed_draw_validation_pipeline->render_passes_to_pipeline.pop(render_pass);
        if (pipeline != indexed_draw_validation_pipeline->render_passes_to_pipeline.end()) {
            DispatchDestroyPipeline(gpuav.device, pipeline->second, nullptr);
        }
    }

    auto *draw_indexed_indirect_index_buffer_pipeline =
        gpuav.shared_resources_manager.TryGet<GraphicsValidationPipeline<DrawIndexedIndirectIndexBufferShader>>();

    if (draw_indexed_indirect_index_buffer_pipeline && draw_indexed_indirect_index_buffer_pipeline->valid) {
        auto pipeline = draw_indexed_indirect_index_buffer_pipeline->render_passes_to_pipeline.pop(render_pass);
        if (pipeline != draw_indexed_indirect_index_buffer_pipeline->render_passes_to_pipeline.end()) {
            DispatchDestroyPipeline(gpuav.device, pipeline->second, nullptr);
        }
    }

    auto *draw_indexed_indirect_vertex_buffer_pipeline =
        gpuav.shared_resources_manager.TryGet<GraphicsValidationPipeline<DrawIndexedIndirectVertexBufferShader>>();

    if (draw_indexed_indirect_vertex_buffer_pipeline && draw_indexed_indirect_vertex_buffer_pipeline->valid) {
        auto pipeline = draw_indexed_indirect_vertex_buffer_pipeline->render_passes_to_pipeline.pop(render_pass);
        if (pipeline != draw_indexed_indirect_vertex_buffer_pipeline->render_passes_to_pipeline.end()) {
            DispatchDestroyPipeline(gpuav.device, pipeline->second, nullptr);
        }
    }
}

}  // namespace draw
}  // namespace gpuav
