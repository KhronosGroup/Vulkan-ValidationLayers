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
#include "gpu/shaders/cmd_validation/draw_push_data.h"
#include "generated/cmd_validation_indexed_draw_vert.h"
#include "generated/cmd_validation_indirect_draw_vert.h"
#include "generated/cmd_validation_mesh_draw_vert.h"

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
                                  const Location &loc, const uint32_t *shader, uint32_t shader_size)
        : device(gpuav.device) {
        VkResult result = VK_SUCCESS;

        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},  // count buffer
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},  // draw buffer
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},  // index buffer
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
            shader_ci.codeSize = shader_size * sizeof(uint32_t);
            shader_ci.pCode = shader;
            shader_ci.pName = "main";
            shader_ci.setLayoutCount = 1u;
            shader_ci.pSetLayouts = &ds_layout;
            shader_ci.pushConstantRangeCount = 1;
            shader_ci.pPushConstantRanges = &push_constant_range;
            result = DispatchCreateShadersEXT(device, 1u, &shader_ci, nullptr, &shader_object);
            if (result != VK_SUCCESS) {
                gpuav.InternalError(device, loc, "Unable to create shader object.");
                return;
            }
        } else {
            VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
            shader_module_ci.codeSize = shader_size * sizeof(uint32_t);
            shader_module_ci.pCode = shader;
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

    bool IsValid() const {
        return (shader_module != VK_NULL_HANDLE || shader_object != VK_NULL_HANDLE) && ds_layout != VK_NULL_HANDLE &&
               pipeline_layout != VK_NULL_HANDLE && device != VK_NULL_HANDLE;
    }
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

struct IndirectInfo {
    IndirectInfo(const vvl::Buffer &buffer_state, VkDeviceSize offset_, size_t struct_size_, uint32_t stride_)
        : buffer(buffer_state.VkHandle()),
          buffer_size(buffer_state.create_info.size),
          offset(offset_),
          range(0),
          stride(stride_),
          max_draw_count(0) {
        // Buffer size must be >= (stride * (drawCount - 1) + offset + sizeof(cmd))

        if (offset < buffer_size) {
            range = buffer_size - offset;
        }
        if (range >= struct_size_) {
            max_draw_count = 1 + uint32_t((range - struct_size_) / stride);
        }
    }

    VkBuffer buffer{};
    VkDeviceSize buffer_size{};
    VkDeviceSize offset{};
    VkDeviceSize range{};
    uint32_t stride{};
    uint32_t max_draw_count{};
};

template <typename Push>
static void SetupIndirectBuffer(Validator &gpuav, const IndirectInfo &indirect_info, uint32_t draw_count, Push &push_constants,
                                std::vector<VkWriteDescriptorSet> &desc_writes, std::vector<VkDescriptorBufferInfo> &buffer_infos) {
    push_constants.flags = glsl::kPreDrawSelectDrawBuffer;
    if (!gpuav.enabled_features.drawIndirectFirstInstance) {
        push_constants.flags |= glsl::kPreDrawSelectFirstInstance;
    }
    push_constants.draw_count = draw_count;
    push_constants.draw_stride = indirect_info.stride / sizeof(uint32_t);

    buffer_infos.emplace_back(VkDescriptorBufferInfo{indirect_info.buffer, indirect_info.offset, VK_WHOLE_SIZE});
    auto desc_write = vku::InitStruct<VkWriteDescriptorSet>();
    desc_write.dstBinding = glsl::kPreDrawIndirectBinding;
    desc_write.descriptorCount = 1;
    desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_writes.push_back(desc_write);
}

template <typename Push>
static void SetupCountBuffer(Validator &gpuav, const IndirectInfo &indirect_info, VkBuffer count_buffer,
                             VkDeviceSize count_buffer_offset, Push &push_constants, std::vector<VkWriteDescriptorSet> &desc_writes,
                             std::vector<VkDescriptorBufferInfo> &buffer_infos) {
    assert(gpuav.phys_dev_props.limits.maxDrawIndirectCount > 0);
    push_constants.flags = glsl::kPreDrawSelectCountBuffer;
    push_constants.prop_count_limit = gpuav.phys_dev_props.limits.maxDrawIndirectCount;
    push_constants.buffer_count_limit = indirect_info.max_draw_count;

    buffer_infos.emplace_back(VkDescriptorBufferInfo{count_buffer, count_buffer_offset, VK_WHOLE_SIZE});
    auto desc_write = vku::InitStruct<VkWriteDescriptorSet>();
    desc_write.dstBinding = glsl::kPreDrawCountBinding;
    desc_write.descriptorCount = 1;
    desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_writes.push_back(desc_write);
}

template <typename Push>
static void DispatchDiagnosticDraw(Validator &gpuav, const Location &loc, CommandBuffer &cb_state, const uint32_t *shader,
                                   uint32_t shader_size, const Push &push_constants, std::vector<VkWriteDescriptorSet> &desc_writes,
                                   std::vector<VkDescriptorBufferInfo> buffer_infos) {
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;
    const bool use_shader_objects = pipeline_state == nullptr;

    auto &shared_draw_resources = gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(
        gpuav, cb_state.GetValidationCmdCommonDescriptorSetLayout(), use_shader_objects, loc, shader, shader_size);

    if (!shared_draw_resources.IsValid()) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Could not create shared draw resources.");
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

    // Insert a draw that can examine some device memory right before the draw we're validating (Pre Draw Validation)
    //
    // NOTE that this validation does not attempt to abort invalid api calls as most other validation does. A crash
    // or DEVICE_LOST resulting from the invalid call will prevent preceeding validation errors from being reported.

    // Save current graphics pipeline state
    RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS);

    for (size_t i = 0; i < buffer_infos.size(); ++i) {
        desc_writes[i].dstSet = draw_validation_desc_set;
        desc_writes[i].pBufferInfo = &buffer_infos[i];
    }
    DispatchUpdateDescriptorSets(gpuav.device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, NULL);
    // Insert diagnostic draw
    if (use_shader_objects) {
        VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
        DispatchCmdBindShadersEXT(cb_state.VkHandle(), 1u, &stage, &shared_draw_resources.shader_object);
    } else {
        DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, validation_pipeline);
    }
    static_assert(sizeof(push_constants) <= 128, "push_constants buffer size >128, need to consider maxPushConstantsSize.");
    DispatchCmdPushConstants(cb_state.VkHandle(), shared_draw_resources.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                             static_cast<uint32_t>(sizeof(push_constants)), &push_constants);
    BindValidationCmdsCommonDescSet(cb_state, VK_PIPELINE_BIND_POINT_GRAPHICS, shared_draw_resources.pipeline_layout,
                                    cb_state.draw_index, static_cast<uint32_t>(cb_state.per_command_error_loggers.size()));
    DispatchCmdBindDescriptorSets(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, shared_draw_resources.pipeline_layout,
                                  glsl::kDiagPerCmdDescriptorSet, 1, &draw_validation_desc_set, 0, nullptr);
    DispatchCmdDraw(cb_state.VkHandle(), 3, 1, 0, 0);
}

void InsertIndirectDrawValidation(Validator &gpuav, const Location &loc, CommandBuffer &cb_state, VkBuffer indirect_buffer,
                                  VkDeviceSize indirect_offset, uint32_t draw_count, VkBuffer count_buffer,
                                  VkDeviceSize count_buffer_offset, uint32_t stride) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    glsl::DrawIndirectPushData push_constants;
    memset(&push_constants, 0, sizeof(push_constants));

    auto buffer_state = gpuav.Get<vvl::Buffer>(indirect_buffer);
    if (!buffer_state) {
        gpuav.InternalError(cb_state.Handle(), loc, "Bad indirect buffer.");
        return;
    }

    const IndirectInfo indirect_info(*buffer_state, indirect_offset, sizeof(VkDrawIndirectCommand), stride);

    std::vector<VkWriteDescriptorSet> desc_writes;
    std::vector<VkDescriptorBufferInfo> buffer_infos;

    SetupIndirectBuffer(gpuav, indirect_info, draw_count, push_constants, desc_writes, buffer_infos);

    if (count_buffer) {
        SetupCountBuffer(gpuav, indirect_info, count_buffer, count_buffer_offset, push_constants, desc_writes, buffer_infos);
    }

    DispatchDiagnosticDraw(gpuav, loc, cb_state, cmd_validation_indirect_draw_vert, cmd_validation_indirect_draw_vert_size,
                           push_constants, desc_writes, buffer_infos);

    CommandBuffer::ErrorLoggerFunc error_logger = [loc, indirect_info](Validator &gpuav, const uint32_t *error_record,
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
                const uint32_t count = error_record[kPreActionParamOffset_0];
                const uint32_t offset =
                    static_cast<uint32_t>(indirect_info.offset);  // TODO: why cast to uin32_t? If it is changed,
                                                                  // think about also doing it in the error message
                const uint32_t draw_size = (indirect_info.stride * (count - 1) + offset + sizeof(VkDrawIndexedIndirectCommand));

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
                                       count, indirect_info.buffer_size, gpuav.FormatHandle(indirect_info.buffer).c_str(),
                                       indirect_info.stride, offset, draw_size);
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
            default:
                break;
        }

        return skip;
    };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}
/*
bufferBindingAddress = buffer[binding].baseAddress + offset[binding];

if (bindingDesc.inputRate == VK_VERTEX_INPUT_RATE_VERTEX)
    effectiveVertexOffset = vertexIndex * stride;
else
    if (divisor == 0)
        effectiveVertexOffset = firstInstance * stride;
    else
        effectiveVertexOffset = (firstInstance + ((instanceIndex - firstInstance) / divisor)) * stride;

attribAddress = bufferBindingAddress + effectiveVertexOffset + attribDesc.offset;

end = size = offset[binding] + (index * stride) + attrib.offset + attrib.size;

end = (size - offset[binding]) = (index * stride) + attrib.offset + attrib.size;

max_index = (size - offset - attrib.offset - attrib.size) / stride
*/
#if 0
static uint32_t MinVertexBufferLen(const vvl::CommandBuffer &cb_state, const vvl::Pipeline &pipeline) {
    uint32_t min_vertex_count = ~0;

    const auto &vertex_attribute_descriptions = pipeline.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT)
                                                    ? cb_state.dynamic_state_value.vertex_attribute_descriptions
                                                    : pipeline.vertex_input_state->vertex_attribute_descriptions;
    // Verify vertex attribute address alignment
    for (uint32_t i = 0; i < vertex_attribute_descriptions.size(); i++) {
        const auto &attribute_description = vertex_attribute_descriptions[i];

        const uint32_t vertex_binding = attribute_description.binding;

        auto it = cb_state.current_vertex_buffer_binding_info.find(vertex_binding);
        if (it == cb_state.current_vertex_buffer_binding_info.cend()) {
            continue;
        }
        const vvl::VertexBufferBinding &vbb = it->second;
            // overall room in the vertex buffer
            VkDeviceSize effective_size = vbb.size - vbb.offset;
        // figure out how many full-stride records fit in the buffer
        auto vertex_count = effective_size / vbb.stride;
        // Check if there still is room to access this attribute at the end
        effective_size -= vertex_count * vbb.stride;
        if (effective_size > 0) {
            const VkDeviceSize attribute_offset = attribute_description.offset;
            const VkFormat attribute_format = attribute_description.format;
        }
    }
    return min_vertex_count;
}
#endif

void InsertIndexedDrawValidation(Validator &gpuav, const Location &loc, CommandBuffer &cb_state, VkBuffer indirect_buffer,
                                 VkDeviceSize indirect_offset, uint32_t draw_count, VkBuffer count_buffer,
                                 VkDeviceSize count_buffer_offset, uint32_t stride, uint32_t first_index, uint32_t index_count) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }
    glsl::DrawIndexedPushData push_constants;
    memset(&push_constants, 0, sizeof(push_constants));

    std::optional<IndirectInfo> indirect_info;
    std::vector<VkWriteDescriptorSet> desc_writes;
    std::vector<VkDescriptorBufferInfo> buffer_infos;

    if (indirect_buffer) {
        auto buffer_state = gpuav.Get<vvl::Buffer>(indirect_buffer);
        if (!buffer_state) {
            gpuav.InternalError(cb_state.Handle(), loc, "Bad indirect buffer.");
            return;
        }
        indirect_info.emplace(*buffer_state, indirect_offset, sizeof(VkDrawIndexedIndirectCommand), stride);
        SetupIndirectBuffer(gpuav, *indirect_info, draw_count, push_constants, desc_writes, buffer_infos);
        if (count_buffer) {
            SetupCountBuffer(gpuav, *indirect_info, count_buffer, count_buffer_offset, push_constants, desc_writes, buffer_infos);
        }
    }
    if (!gpuav.enabled_features.robustBufferAccess2 && cb_state.index_buffer_binding.buffer) {
        push_constants.flags |= glsl::kPreDrawSelectIndexBuffer;

        // direct draw parameters will be ignored for indirect via flags
        push_constants.first_index = first_index;
        push_constants.index_count = index_count;
        push_constants.vertex_offset = 0;

        switch (cb_state.index_buffer_binding.index_type) {
            case VK_INDEX_TYPE_UINT32:
                push_constants.index_width = 32;
                break;
            case VK_INDEX_TYPE_UINT16:
                push_constants.index_width = 16;
                break;
            case VK_INDEX_TYPE_UINT8_KHR:
                push_constants.index_width = 8;
                break;
            default:
                gpuav.InternalError(cb_state.Handle(), loc, "Unsupported indexType");
                return;
        }
        push_constants.vertex_buffer_size = 3;  // TODO vertex size calc

        buffer_infos.emplace_back(VkDescriptorBufferInfo{cb_state.index_buffer_binding.buffer, cb_state.index_buffer_binding.offset,
                                                         cb_state.index_buffer_binding.size});
        auto desc_write = vku::InitStruct<VkWriteDescriptorSet>();
        desc_write.dstBinding = glsl::kPreDrawIndexBinding;
        desc_write.descriptorCount = 1;
        desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes.push_back(desc_write);
    }

    DispatchDiagnosticDraw(gpuav, loc, cb_state, cmd_validation_indexed_draw_vert, cmd_validation_indexed_draw_vert_size,
                           push_constants, desc_writes, buffer_infos);

    CommandBuffer::ErrorLoggerFunc error_logger = [loc, indirect_info](Validator &gpuav, const uint32_t *error_record,
                                                                       const LogObjectList &objlist) {
        bool skip = false;

        using namespace glsl;
        auto group = error_record[kHeaderErrorGroupOffset];
        auto subcode = error_record[kHeaderErrorSubCodeOffset];

        if (group != kErrorGroupGpuPreDraw) {
            assert(false);
            return skip;
        }

        const GpuVuid &vuids = GetGpuVuid(loc.function);

        switch (subcode) {
            case kErrorSubCodePreDrawBufferSize: {
                // Buffer size must be >= (stride * (drawCount - 1) + offset + sizeof(VkDrawIndexedIndirectCommand))
                const uint32_t count = error_record[kPreActionParamOffset_0];
                const uint32_t offset =
                    static_cast<uint32_t>(indirect_info->offset);  // TODO: why cast to uin32_t? If it is changed,
                                                                   // think about also doing it in the error message
                const uint32_t draw_size = (indirect_info->stride * (count - 1) + offset + sizeof(VkDrawIndexedIndirectCommand));

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
                                       count, indirect_info->buffer_size, gpuav.FormatHandle(indirect_info->buffer).c_str(),
                                       indirect_info->stride, offset, draw_size);
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
            case kErrorSubCodePreDrawIndexBuffer: {
                const uint32_t first_index = error_record[kPreActionParamOffset_0];
                const uint32_t index_count = error_record[kPreActionParamOffset_1];
                gpuav.LogError(vuids.index_buffer_size, objlist, loc,
                               "The robustBufferAccess2 feature is not enabled, but the firstIndex = %" PRIu32
                               " plus indexCount = %" PRIu32
                               " fields of a VkDrawIndexedIndirectCommand structure exceed the bounds of the index buffer",
                               first_index, index_count);
                break;
            }
            case kErrorSubCodePreDrawVertexIndex: {
                const uint32_t index = error_record[kPreActionParamOffset_0];
                const uint32_t vertex_index = error_record[kPreActionParamOffset_1];
                gpuav.LogError(vuids.vertex_index_oob, objlist, loc,
                               "The robustBufferAccess2 feature is not enabled, but the value in the index buffer at index %" PRIu32
                               " is %" PRIu32 ", which is out of bounds of a vertex buffer",
                               index, vertex_index);
                break;
            }
            default:
                break;
        }

        return skip;
    };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}

void InsertMeshDrawValidation(Validator &gpuav, const Location &loc, CommandBuffer &cb_state, VkBuffer indirect_buffer,
                              VkDeviceSize indirect_offset, uint32_t draw_count, VkBuffer count_buffer,
                              VkDeviceSize count_buffer_offset, uint32_t stride) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    auto buffer_state = gpuav.Get<vvl::Buffer>(indirect_buffer);
    if (!buffer_state) {
        gpuav.InternalError(cb_state.Handle(), loc, "Bad indirect buffer.");
        return;
    }

    const IndirectInfo indirect_info(*buffer_state, indirect_offset, sizeof(VkDrawMeshTasksIndirectCommandEXT), stride);

    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkWriteDescriptorSet> desc_writes;
    glsl::DrawMeshPushData push_constants;

    SetupIndirectBuffer(gpuav, indirect_info, draw_count, push_constants, desc_writes, buffer_infos);
    if (count_buffer) {
        SetupCountBuffer(gpuav, indirect_info, count_buffer, count_buffer_offset, push_constants, desc_writes, buffer_infos);
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;

    bool emit_task_error = false;
    const VkShaderStageFlags stages = pipeline_state->create_info_shaders;
    push_constants.draw_count = count_buffer ? 0 : draw_count;
    push_constants.draw_stride = stride / sizeof(uint32_t);
    if (stages & VK_SHADER_STAGE_TASK_BIT_EXT) {
        emit_task_error = true;
        push_constants.max_workgroup_count_x = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0];
        push_constants.max_workgroup_count_y = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[1];
        push_constants.max_workgroup_count_z = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[2];
        push_constants.max_workgroup_total_count = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupTotalCount;
    } else {
        push_constants.max_workgroup_count_x = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0];
        push_constants.max_workgroup_count_y = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1];
        push_constants.max_workgroup_count_z = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2];
        push_constants.max_workgroup_total_count = gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount;
    }

    buffer_infos.emplace_back(VkDescriptorBufferInfo{indirect_buffer, indirect_offset, VK_WHOLE_SIZE});

    auto desc_write = vku::InitStruct<VkWriteDescriptorSet>();
    desc_write.dstBinding = glsl::kPreDrawIndirectBinding;
    desc_write.descriptorCount = 1;
    desc_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_writes.push_back(desc_write);

    DispatchDiagnosticDraw(gpuav, loc, cb_state, cmd_validation_mesh_draw_vert, cmd_validation_mesh_draw_vert_size, push_constants,
                           desc_writes, buffer_infos);

    CommandBuffer::ErrorLoggerFunc error_logger = [loc, indirect_info, emit_task_error](Validator &gpuav,
                                                                                        const uint32_t *error_record,
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
                const uint32_t offset =
                    static_cast<uint32_t>(indirect_info.offset);  // TODO: why cast to uin32_t? If it is changed,
                                                                  // think about also doing it in the error message
                const uint32_t draw_size =
                    (indirect_info.stride * (count - 1) + offset + sizeof(VkDrawMeshTasksIndirectCommandEXT));

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
                                       " (stride * (drawCount - 1) + offset + sizeof(VkDrawMeshTasksIndirectCommandEXT)) = %" PRIu32
                                       ".",
                                       count, indirect_info.buffer_size, gpuav.FormatHandle(indirect_info.buffer).c_str(),
                                       indirect_info.stride, offset, draw_size);
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
