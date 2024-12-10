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
#include "gpu/resources/gpuav_vulkan_objects.h"
#include "gpu/resources/gpuav_state_trackers.h"
#include "gpu/shaders/gpuav_error_header.h"
#include "gpu/shaders/gpuav_shaders_constants.h"

#include "state_tracker/render_pass_state.h"
#include "gpu/shaders/gpuav_error_header.h"
#include "gpu/shaders/gpuav_shaders_constants.h"
#include "gpu/shaders/cmd_validation/draw_push_data.h"
#include "generated/cmd_validation_draw_mesh_indirect_comp.h"
#include "generated/cmd_validation_first_instance_comp.h"
#include "generated/cmd_validation_count_buffer_comp.h"
#include "generated/cmd_validation_draw_indexed_comp.h"
#include "generated/cmd_validation_draw_indexed_indirect_index_buffer_comp.h"
#include "generated/cmd_validation_draw_indexed_indirect_vertex_buffer_comp.h"

#include "profiling/profiling.h"

#include <optional>

namespace gpuav {
namespace valcmd {

struct SharedDrawValidationResources {
    vko::Buffer dummy_buffer;  // Used to fill unused buffer bindings in validation pipelines
    bool valid = false;

    SharedDrawValidationResources(Validator &gpuav, const Location &loc) : dummy_buffer(gpuav) {
        VkBufferCreateInfo dummy_buffer_info = vku::InitStructHelper();
        dummy_buffer_info.size = 64;// whatever
        dummy_buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        dummy_buffer_info.size = dummy_buffer_info.size;
        const bool success = dummy_buffer.Create(loc, &dummy_buffer_info, &alloc_info);
        if (!success) {
            valid = false;
            return;
        }

        valid = true;
    }

    ~SharedDrawValidationResources() { dummy_buffer.Destroy(); }
};

struct BoundStorageBuffer {
    uint32_t binding = vvl::kU32Max;
    VkDescriptorBufferInfo info{VK_NULL_HANDLE, vvl::kU64Max, 0};
};

template <typename ShaderResources>
struct ComputeValidationPipeline {
    ComputeValidationPipeline(Validator &gpuav, const Location &loc, VkDescriptorSetLayout error_output_desc_set_layout) {
        std::vector<VkDescriptorSetLayoutBinding> specific_bindings = ShaderResources::GetDescriptorSetLayoutBindings();

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = sizeof(ShaderResources::push_constants);  // 0 size is ok here

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
        shader_module_ci.codeSize = ShaderResources::GetSpirvSize();
        shader_module_ci.pCode = ShaderResources::GetSpirv();
        result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &shader_module);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create shader module.");
            return;
        }

        VkComputePipelineCreateInfo compute_validation_pipeline_ci = vku::InitStructHelper();
        compute_validation_pipeline_ci.stage = vku::InitStructHelper();
        compute_validation_pipeline_ci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        compute_validation_pipeline_ci.stage.module = shader_module;
        compute_validation_pipeline_ci.stage.pName = "main";
        compute_validation_pipeline_ci.layout = pipeline_layout;
        result = DispatchCreateComputePipelines(device, VK_NULL_HANDLE, 1, &compute_validation_pipeline_ci, nullptr, &pipeline);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(device, loc, "Unable to create compute validation pipeline.");
            return;
        }

        valid = true;
    }

    ~ComputeValidationPipeline() {
        if (pipeline != VK_NULL_HANDLE) {
            DispatchDestroyPipeline(device, pipeline, nullptr);
        }

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

    void BindShaderResources(Validator &gpuav, CommandBuffer &cb_state, uint32_t cmd_index, uint32_t error_logger_index,
                             const ShaderResources &shader_resources) {
        // Error logging resources
        BindErrorLoggingDescSet(gpuav, cb_state, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, cmd_index, error_logger_index);

        // Specific resources
        VkDescriptorSet desc_set = cb_state.gpu_resources_manager.GetManagedDescriptorSet(specific_desc_set_layout);
        std::vector<VkWriteDescriptorSet> desc_writes = shader_resources.GetDescriptorWrites(desc_set);
        DispatchUpdateDescriptorSets(gpuav.device, uint32_t(desc_writes.size()), desc_writes.data(), 0, nullptr);

        DispatchCmdPushConstants(cb_state.VkHandle(), pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                 sizeof(shader_resources.push_constants), &shader_resources.push_constants);

        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout,
                                      shader_resources.desc_set_id, 1, &desc_set, 0, nullptr);
    }

    VkDevice device = VK_NULL_HANDLE;
    VkDescriptorSetLayout specific_desc_set_layout = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkShaderModule shader_module = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    bool valid = false;
};

void FlushValidationCmds(Validator &gpuav, CommandBuffer &cb_state) {
    VVL_TracyPlot("gpuav::valcmd::FlushValidationCmds", int64_t(cb_state.per_render_pass_validation_commands.size()));
    VVL_ZoneScoped;
    RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    for (CommandBuffer::ValidationCommandFunc &validation_cmd : cb_state.per_render_pass_validation_commands) {
        validation_cmd(gpuav, cb_state);
    }
    cb_state.per_render_pass_validation_commands.clear();
}

struct FirstInstanceValidationShader {
    static size_t GetSpirvSize() { return cmd_validation_first_instance_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return cmd_validation_first_instance_comp; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::FirstInstancePushData push_constants{};
    BoundStorageBuffer draw_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndirectBuffer};
    BoundStorageBuffer count_buffer_binding = {gpuav::glsl::kPreDrawBinding_CountBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_IndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},  // indirect buffer
            {gpuav::glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},  // count buffer
        };

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) const {
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
                   uint32_t first_instance_member_pos, uint32_t draws_count, VkBuffer count_buffer,
                   VkDeviceSize count_buffer_offset, const char *vuid) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    if (gpuav.enabled_features.drawIndirectFirstInstance) return;

    CommandBuffer::ValidationCommandFunc validation_cmd = [draw_buffer, draw_buffer_offset, draw_cmds_byte_stride,
                                                           first_instance_member_pos, draws_count, count_buffer,
                                                           count_buffer_offset, draw_i = cb_state.draw_index,
                                                           error_logger_i = uint32_t(cb_state.per_command_error_loggers.size()),
                                                           loc](Validator &gpuav, CommandBuffer &cb_state) {
        SharedDrawValidationResources &shared_draw_validation_resources =
            gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
        if (!shared_draw_validation_resources.valid) return;
        ComputeValidationPipeline<FirstInstanceValidationShader> &validation_pipeline =
            gpuav.shared_resources_manager.Get<ComputeValidationPipeline<FirstInstanceValidationShader>>(
                gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
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

            shader_resources.draw_buffer_binding.info = {draw_buffer, 0, VK_WHOLE_SIZE};
            shader_resources.push_constants.draw_buffer_dwords_offset = (uint32_t)draw_buffer_offset / sizeof(uint32_t);
            if (count_buffer) {
                shader_resources.push_constants.flags |= gpuav::glsl::kFirstInstanceFlags_DrawCountFromBuffer;
                shader_resources.count_buffer_binding.info = {count_buffer, 0, sizeof(uint32_t) + count_buffer_offset};
                shader_resources.push_constants.count_buffer_dwords_offset = (uint32_t)count_buffer_offset / sizeof(uint32_t);

            } else {
                shader_resources.count_buffer_binding.info = {shared_draw_validation_resources.dummy_buffer.VkHandle(), 0,
                                                              VK_WHOLE_SIZE};
            }

            validation_pipeline.BindShaderResources(gpuav, cb_state, draw_i, error_logger_i, shader_resources);
        }

        // Setup validation pipeline
        // ---
        {
            DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

            uint32_t max_held_draw_cmds = 0;
            if (draw_buffer_state->create_info.size > draw_buffer_offset) {
                max_held_draw_cmds =
                    static_cast<uint32_t>((draw_buffer_state->create_info.size - draw_buffer_offset) / draw_cmds_byte_stride);
            }
            // It is assumed that the number of draws to validate is fairly low.
            // Otherwise might reconsider having a warp dimension of (1, 1, 1)
            // Maybe another reason to add telemetry?
            const uint32_t work_group_count = std::min(draws_count, max_held_draw_cmds);

            if (work_group_count == 0) {
                return;
            }

            VVL_TracyPlot("gpuav::valcmd::FirstInstance Dispatch size", int64_t(work_group_count));
            DispatchCmdDispatch(cb_state.VkHandle(), work_group_count, 1, 1);

            // synchronize draw buffer validation (read) against subsequent writes
            std::array<VkBufferMemoryBarrier, 2> buffer_memory_barriers = {};
            uint32_t buffer_memory_barriers_count = 1;
            buffer_memory_barriers[0] = vku::InitStructHelper();
            buffer_memory_barriers[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            buffer_memory_barriers[0].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            buffer_memory_barriers[0].buffer = draw_buffer;
            buffer_memory_barriers[0].offset = draw_buffer_offset;
            buffer_memory_barriers[0].size = work_group_count * sizeof(uint32_t);

            if (count_buffer) {
                buffer_memory_barriers[1] = vku::InitStructHelper();
                buffer_memory_barriers[1].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                buffer_memory_barriers[1].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                buffer_memory_barriers[1].buffer = count_buffer;
                buffer_memory_barriers[1].offset = count_buffer_offset;
                buffer_memory_barriers[1].size = sizeof(uint32_t);
                ++buffer_memory_barriers_count;
            }

            DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, buffer_memory_barriers_count,
                                       buffer_memory_barriers.data(), 0, nullptr);
        }
    };

    cb_state.per_render_pass_validation_commands.emplace_back(std::move(validation_cmd));

    // Register error logger. Happens per command GPU-AV intercepts
    // ---
    CommandBuffer::ErrorLoggerFunc error_logger =
        [loc, vuid, draw_indirect_struct_name](Validator &gpuav, const CommandBuffer &, const uint32_t *error_record,
                                               const LogObjectList &objlist, const std::vector<std::string> &initial_label_stack) {
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
    static size_t GetSpirvSize() { return cmd_validation_count_buffer_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return cmd_validation_count_buffer_comp; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::CountBufferPushData push_constants{};
    BoundStorageBuffer count_buffer_binding = {gpuav::glsl::kPreDrawBinding_CountBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},  // count buffer
        };

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) const {
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

    auto draw_buffer_state = gpuav.Get<vvl::Buffer>(draw_buffer);
    if (!draw_buffer_state) {
        gpuav.InternalError(LogObjectList(cb_state.VkHandle(), draw_buffer), loc, "buffer must be a valid VkBuffer handle");
        return;
    }

    CommandBuffer::ValidationCommandFunc validation_cmd = [draw_buffer_size = draw_buffer_state->create_info.size,
                                                           draw_buffer_offset, draw_indirect_struct_byte_size,
                                                           draw_cmds_byte_stride, count_buffer, count_buffer_offset,
                                                           draw_i = cb_state.draw_index,
                                                           error_logger_i = uint32_t(cb_state.per_command_error_loggers.size()),
                                                           loc](Validator &gpuav, CommandBuffer &cb_state) {
        SharedDrawValidationResources &shared_draw_validation_resources =
            gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
        if (!shared_draw_validation_resources.valid) return;
        ComputeValidationPipeline<CountBufferValidationShader> &validation_pipeline =
            gpuav.shared_resources_manager.Get<ComputeValidationPipeline<CountBufferValidationShader>>(
                gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
        if (!validation_pipeline.valid) return;

        // Setup shader resources
        // ---
        {
            CountBufferValidationShader shader_resources;
            shader_resources.push_constants.draw_cmds_byte_stride = draw_cmds_byte_stride;
            shader_resources.push_constants.draw_buffer_offset = draw_buffer_offset;
            shader_resources.push_constants.draw_buffer_size = draw_buffer_size;
            shader_resources.push_constants.draw_cmd_byte_size = draw_indirect_struct_byte_size;
            shader_resources.push_constants.device_limit_max_draw_indirect_count = gpuav.phys_dev_props.limits.maxDrawIndirectCount;

            shader_resources.count_buffer_binding.info = {count_buffer, 0, count_buffer_offset + sizeof(uint32_t)};
            shader_resources.push_constants.count_buffer_dwords_offset = (uint32_t)count_buffer_offset / sizeof(uint32_t);

            validation_pipeline.BindShaderResources(gpuav, cb_state, draw_i, error_logger_i, shader_resources);
        }

        // Setup validation pipeline
        // ---
        {
            DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);
            DispatchCmdDispatch(cb_state.VkHandle(), 1, 1, 1);
            // synchronize draw buffer validation (read) against subsequent writes
            VkBufferMemoryBarrier count_buffer_memory_barrier = vku::InitStructHelper();
            count_buffer_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            count_buffer_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            count_buffer_memory_barrier.buffer = count_buffer;
            count_buffer_memory_barrier.offset = count_buffer_offset;
            count_buffer_memory_barrier.size = sizeof(uint32_t);

            DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &count_buffer_memory_barrier, 0,
                                       nullptr);
        }
    };

    cb_state.per_render_pass_validation_commands.emplace_back(std::move(validation_cmd));

    // Register error logger
    // ---
    CommandBuffer::ErrorLoggerFunc error_logger = [loc, draw_buffer, draw_buffer_size = draw_buffer_state->create_info.size,
                                                   draw_buffer_offset, draw_indirect_struct_byte_size, draw_cmds_byte_stride,
                                                   draw_indirect_struct_name, vuid_draw_buffer_size,
                                                   vuid_max_draw_count](Validator &gpuav, const CommandBuffer &cb_state,
                                                                        const uint32_t *error_record, const LogObjectList &objlist,
                                                                        const std::vector<std::string> &initial_label_stack) {
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
    static size_t GetSpirvSize() { return cmd_validation_draw_mesh_indirect_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return cmd_validation_draw_mesh_indirect_comp; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::DrawMeshPushData push_constants{};
    BoundStorageBuffer draw_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndirectBuffer};
    BoundStorageBuffer count_buffer_binding = {gpuav::glsl::kPreDrawBinding_CountBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_IndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},  // indirect buffer
            {gpuav::glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},  // count buffer
        };

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) const {
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
                      VkDeviceSize count_buffer_offset, uint32_t draw_count) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    auto draw_buffer_state = gpuav.Get<vvl::Buffer>(draw_buffer);
    if (!draw_buffer_state) {
        gpuav.InternalError(LogObjectList(cb_state.VkHandle(), draw_buffer), loc, "buffer must be a valid VkBuffer handle");
        return;
    }

    const LvlBindPoint lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const LastBound &last_bound = cb_state.lastBound[lv_bind_point];
    const vvl::Pipeline *pipeline_state = last_bound.pipeline_state;
    const VkShaderStageFlags stages = pipeline_state->create_info_shaders;
    const bool is_task_shader = (stages & VK_SHADER_STAGE_TASK_BIT_EXT) == VK_SHADER_STAGE_TASK_BIT_EXT;

    CommandBuffer::ValidationCommandFunc validation_cmd =
        [draw_buffer, draw_buffer_size = draw_buffer_state->create_info.size, draw_buffer_offset, draw_cmds_byte_stride,
         count_buffer, count_buffer_offset, draw_count, is_task_shader, draw_i = cb_state.draw_index,
         error_logger_i = uint32_t(cb_state.per_command_error_loggers.size()), loc](Validator &gpuav, CommandBuffer &cb_state) {
            SharedDrawValidationResources &shared_draw_validation_resources =
                gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
            if (!shared_draw_validation_resources.valid) return;
            ComputeValidationPipeline<MeshValidationShader> &validation_pipeline =
                gpuav.shared_resources_manager.Get<ComputeValidationPipeline<MeshValidationShader>>(
                    gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
            if (!validation_pipeline.valid) return;

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

                shader_resources.draw_buffer_binding.info = {draw_buffer, 0, VK_WHOLE_SIZE};
                shader_resources.push_constants.draw_buffer_dwords_offset = (uint32_t)draw_buffer_offset / sizeof(uint32_t);
                if (count_buffer != VK_NULL_HANDLE) {
                    shader_resources.push_constants.flags |= glsl::kDrawMeshFlags_DrawCountFromBuffer;
                    shader_resources.count_buffer_binding.info = {count_buffer, 0, count_buffer_offset + sizeof(uint32_t)};
                    shader_resources.push_constants.count_buffer_dwords_offset = (uint32_t)count_buffer_offset / sizeof(uint32_t);
                } else {
                    shader_resources.count_buffer_binding.info = {shared_draw_validation_resources.dummy_buffer.VkHandle(), 0,
                                                                  VK_WHOLE_SIZE};
                }

                validation_pipeline.BindShaderResources(gpuav, cb_state, draw_i, error_logger_i, shader_resources);
            }

            // Setup validation pipeline
            // ---
            {
                DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

                uint32_t max_held_draw_cmds = 0;
                if (draw_buffer_size > draw_buffer_offset) {
                    max_held_draw_cmds = static_cast<uint32_t>((draw_buffer_size - draw_buffer_offset) / draw_cmds_byte_stride);
                }
                const uint32_t work_group_count = std::min(draw_count, max_held_draw_cmds);
                VVL_TracyPlot("gpuav::valcmd::DrawMeshIndirect Dispatch size", int64_t(work_group_count));
                DispatchCmdDispatch(cb_state.VkHandle(), work_group_count, 1, 1);

                // synchronize draw buffer validation (read) against subsequent writes
                std::array<VkBufferMemoryBarrier, 2> buffer_memory_barriers = {};
                uint32_t buffer_memory_barriers_count = 1;
                buffer_memory_barriers[0] = vku::InitStructHelper();
                buffer_memory_barriers[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                buffer_memory_barriers[0].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                buffer_memory_barriers[0].buffer = draw_buffer;
                buffer_memory_barriers[0].offset = draw_buffer_offset;
                buffer_memory_barriers[0].size = work_group_count * sizeof(uint32_t);

                if (count_buffer) {
                    buffer_memory_barriers[1] = vku::InitStructHelper();
                    buffer_memory_barriers[1].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    buffer_memory_barriers[1].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                    buffer_memory_barriers[1].buffer = count_buffer;
                    buffer_memory_barriers[1].offset = count_buffer_offset;
                    buffer_memory_barriers[1].size = sizeof(uint32_t);
                    ++buffer_memory_barriers_count;
                }

                DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, buffer_memory_barriers_count,
                                           buffer_memory_barriers.data(), 0, nullptr);
            }
        };
    cb_state.per_render_pass_validation_commands.emplace_back(std::move(validation_cmd));

    // Register error logger
    // ---
    CommandBuffer::ErrorLoggerFunc error_logger = [loc, is_task_shader](Validator &gpuav, const CommandBuffer &cb_state,
                                                                        const uint32_t *error_record, const LogObjectList &objlist,
                                                                        const std::vector<std::string> &initial_label_stack) {
        bool skip = false;

        using namespace glsl;

        const char *vuid_task_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07322";
        const char *vuid_task_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07323";
        const char *vuid_task_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07324";
        const char *vuid_task_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07325";
        const char *vuid_mesh_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07326";
        const char *vuid_mesh_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07327";
        const char *vuid_mesh_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07328";
        const char *vuid_mesh_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329";

        const uint32_t draw_i = error_record[kPreActionParamOffset_1];
        const char *group_count_name = is_task_shader ? "maxTaskWorkGroupCount" : "maxMeshWorkGroupCount";
        const char *group_count_total_name = is_task_shader ? "maxTaskWorkGroupTotalCount" : "maxMeshWorkGroupTotalCount";

        switch (error_record[kHeaderErrorSubCodeOffset]) {
            case kErrorSubCodePreDrawGroupCountX: {
                const char *vuid_group_count_exceeds_max =
                    is_task_shader ? vuid_task_group_count_exceeds_max_x : vuid_mesh_group_count_exceeds_max_x;
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
                    is_task_shader ? vuid_task_group_count_exceeds_max_y : vuid_mesh_group_count_exceeds_max_y;
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
                    is_task_shader ? vuid_task_group_count_exceeds_max_z : vuid_mesh_group_count_exceeds_max_z;
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
                    is_task_shader ? vuid_task_group_count_exceeds_max_total : vuid_mesh_group_count_exceeds_max_total;
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

struct SmallestVertexBufferBinding {
    VkDeviceSize smallest_vertex_attributes_count = std::numeric_limits<VkDeviceSize>::max();
    uint32_t binding = std::numeric_limits<uint32_t>::max();
    vvl::VertexBufferBinding binding_info{};
    struct Attribute {
        uint32_t location{};
        uint32_t binding{};
        VkFormat format{};
        uint32_t offset{};
    } attribute;
};
// Computes the smallest vertex attributes count among the set of bound vertex buffers.
// Used to detect out of bounds indices in index buffers.
// If no vertex buffer is bound, smallest_vertex_attributes_count is std::numeric_limits<uint32_t>::max()
// indicating that no index can be out of bound
static std::optional<SmallestVertexBufferBinding> SmallestVertexAttributesCount(const vvl::CommandBuffer &cb_state) {
    const LvlBindPoint lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const LastBound &last_bound = cb_state.lastBound[lv_bind_point];
    const vvl::Pipeline *pipeline_state = last_bound.pipeline_state;
    const bool use_shader_objects = pipeline_state == nullptr;

    const bool dynamic_vertex_input = use_shader_objects || pipeline_state->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT);

    const auto &vertex_binding_descriptions =
        dynamic_vertex_input ? cb_state.dynamic_state_value.vertex_bindings : pipeline_state->vertex_input_state->bindings;

    std::optional<SmallestVertexBufferBinding> smallest_vertex_buffer_binding;

    for (const auto &[binding, vertex_binding_desc] : vertex_binding_descriptions) {
        auto find_vbb = cb_state.current_vertex_buffer_binding_info.find(binding);
        if (find_vbb == cb_state.current_vertex_buffer_binding_info.cend()) {
            // This is a validation error
            continue;
        }

        const vvl::VertexBufferBinding &vbb = find_vbb->second;

        // #ARNO_TODO: Should I only loop over vertex attributes actually used by bound pipelines,
        // according to its vertex shader?
        for (const auto &[Location, attrib] : vertex_binding_desc.locations) {
            const VkDeviceSize attribute_size = vkuFormatElementSize(attrib.desc.format);

            const VkDeviceSize stride =
                vbb.stride != 0 ? vbb.stride : attribute_size;  // Tracked stride should already handle all possible value origin

            VkDeviceSize vertex_buffer_remaining_size = vbb.size > attrib.desc.offset ? vbb.size - attrib.desc.offset : 0;

            VkDeviceSize vertex_attributes_count = vertex_buffer_remaining_size / stride;
            if (vertex_buffer_remaining_size > vertex_attributes_count * stride) {
                vertex_buffer_remaining_size -= vertex_attributes_count * stride;
            } else {
                vertex_buffer_remaining_size = 0;
            }

            // maybe room for one more attribute but not full stride - not having stride space does not matter for last element
            if (vertex_buffer_remaining_size >= attribute_size) {
                vertex_attributes_count += 1;
            }

            if (!smallest_vertex_buffer_binding.has_value()) {
                smallest_vertex_buffer_binding = SmallestVertexBufferBinding{};
            }
            smallest_vertex_buffer_binding->smallest_vertex_attributes_count =
                std::min(smallest_vertex_buffer_binding->smallest_vertex_attributes_count, vertex_attributes_count);
            if (smallest_vertex_buffer_binding->smallest_vertex_attributes_count == vertex_attributes_count) {
                smallest_vertex_buffer_binding->binding = binding;
                smallest_vertex_buffer_binding->binding_info = vbb;
                smallest_vertex_buffer_binding->attribute.location = attrib.desc.location;
                smallest_vertex_buffer_binding->attribute.binding = attrib.desc.binding;
                smallest_vertex_buffer_binding->attribute.format = attrib.desc.format;
                smallest_vertex_buffer_binding->attribute.offset = attrib.desc.offset;
            }
        }
    }
    return smallest_vertex_buffer_binding;
}

struct IndexBufferValidationShader {
    static size_t GetSpirvSize() { return cmd_validation_draw_indexed_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return cmd_validation_draw_indexed_comp; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::IndexedDrawPushData push_constants{};
    BoundStorageBuffer index_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndexBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_IndexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) const {
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

void DrawIndexed(Validator &gpuav, CommandBuffer &cb_state, const Location &loc, uint32_t index_count, uint32_t first_index,
                 uint32_t vertex_offset, const char *vuid_oob_vertex) {
    if (!gpuav.gpuav_settings.validate_index_buffers) {
        return;
    }

    if (gpuav.enabled_features.robustBufferAccess2) {
        const LvlBindPoint lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
        const LastBound &last_bound = cb_state.lastBound[lv_bind_point];
        const vvl::Pipeline *pipeline_state = last_bound.pipeline_state;
        if (pipeline_state) {
            const auto robustness_ci =
                vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfo>(pipeline_state->GraphicsCreateInfo().pNext);
            if (robustness_ci && robustness_ci->vertexInputs) {
                return;
            }
        }
    }

    if (!cb_state.IsPrimary()) {
        // TODO Unhandled for now. Potential issues with accessing the right vertex buffers
        // in secondary command buffers
        return;
    }

    if (!cb_state.index_buffer_binding.buffer) {
        return;
    }

    const std::optional<SmallestVertexBufferBinding> smallest_vertex_buffer_binding = SmallestVertexAttributesCount(cb_state);
    if (!smallest_vertex_buffer_binding.has_value()) {
        // cannot overrun index buffer, skip validation
        return;
    }

    CommandBuffer::ValidationCommandFunc validation_cmd = [index_buffer_binding = cb_state.index_buffer_binding, index_count,
                                                           first_index, vertex_offset,
                                                           smallest_vertex_buffer_binding = *smallest_vertex_buffer_binding,
                                                           draw_i = cb_state.draw_index,
                                                           error_logger_i = uint32_t(cb_state.per_command_error_loggers.size()),
                                                           loc](Validator &gpuav, CommandBuffer &cb_state) {
        SharedDrawValidationResources &shared_draw_validation_resources =
            gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
        if (!shared_draw_validation_resources.valid) return;
        ComputeValidationPipeline<IndexBufferValidationShader> &validation_pipeline =
            gpuav.shared_resources_manager.Get<ComputeValidationPipeline<IndexBufferValidationShader>>(
                gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
        if (!validation_pipeline.valid) return;

        const uint32_t index_bits_size = GetIndexBitsSize(index_buffer_binding.index_type);
        const uint32_t max_indices_in_buffer = static_cast<uint32_t>(index_buffer_binding.size / (index_bits_size / 8u));
        const uint32_t indices_to_check = std::min(index_count, max_indices_in_buffer);

        {
            IndexBufferValidationShader shader_resources;
            shader_resources.push_constants.smallest_vertex_attributes_count =
                static_cast<uint32_t>(smallest_vertex_buffer_binding.smallest_vertex_attributes_count);
            shader_resources.push_constants.index_width = index_bits_size;
            shader_resources.push_constants.vertex_offset = vertex_offset;
            shader_resources.push_constants.first_index = first_index;

            shader_resources.index_buffer_binding.info = {index_buffer_binding.buffer, 0,
                                                          index_buffer_binding.offset + index_buffer_binding.size};
            shader_resources.push_constants.index_buffer_dwords_offset = (uint32_t)index_buffer_binding.offset / sizeof(uint32_t);
            shader_resources.push_constants.indices_to_check = indices_to_check;

            validation_pipeline.BindShaderResources(gpuav, cb_state, draw_i, error_logger_i, shader_resources);
        }

        {
            DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

            // Do not overrun index buffer
            constexpr uint32_t work_group_size = 64;
            const uint32_t work_group_count =
                indices_to_check / work_group_size + uint32_t(indices_to_check % work_group_size != 0);
            VVL_TracyPlot("gpuav::valcmd::DrawIndexed Dispatch size", int64_t(work_group_count));
            DispatchCmdDispatch(cb_state.VkHandle(), work_group_count, 1, 1);
            // synchronize draw buffer validation (read) against subsequent writes
            VkBufferMemoryBarrier index_buffer_memory_barrier = vku::InitStructHelper();
            index_buffer_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            index_buffer_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            index_buffer_memory_barrier.buffer = index_buffer_binding.buffer;
            index_buffer_memory_barrier.offset = index_buffer_binding.offset;
            index_buffer_memory_barrier.size = index_buffer_binding.size;

            DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &index_buffer_memory_barrier, 0,
                                       nullptr);
        }
    };
    cb_state.per_render_pass_validation_commands.emplace_back(std::move(validation_cmd));

    CommandBuffer::ErrorLoggerFunc error_logger =
        [loc, vuid_oob_vertex, smallest_vertex_buffer_binding = *smallest_vertex_buffer_binding,
         index_buffer_binding = cb_state.index_buffer_binding](Validator &gpuav, const CommandBuffer &cb_state,
                                                               const uint32_t *error_record, const LogObjectList &objlist,
                                                               const std::vector<std::string> &initial_label_stack) {
            bool skip = false;

            using namespace glsl;

            switch (error_record[kHeaderErrorSubCodeOffset]) {
                case kErrorSubCode_OobVertexBuffer: {
                    const uint32_t index_buffer_offset = error_record[kPreActionParamOffset_0];
                    const int32_t vertex_offset = static_cast<int32_t>(error_record[kPreActionParamOffset_1]);
                    const uint32_t vertex_index = error_record[kPreActionParamOffset_2];
                    const uint32_t index_buffer_value = static_cast<uint32_t>(int32_t(vertex_index) - vertex_offset);
                    const uint32_t index_bits_size = GetIndexBitsSize(index_buffer_binding.index_type);
                    const uint32_t max_indices_in_buffer =
                        static_cast<uint32_t>(index_buffer_binding.size / (index_bits_size / 8u));

                    gpuav.LogError(
                        vuid_oob_vertex, objlist, loc,
                        "Vertex index %" PRIu32
                        " is not within the smallest bound vertex buffer.\n"
                        "index_buffer[ %" PRIu32 " ] (%" PRIu32 ") + vertexOffset (%" PRIi32 ") = Vertex index %" PRIu32
                        "\n"

                        "Smallest vertex buffer binding info, causing OOB access:\n"
                        "- Buffer: %s\n"
                        "- Binding: %" PRIu32
                        "\n"
                        "- Binding size (effective): %" PRIu64
                        " bytes\n"
                        "- Binding offset: %" PRIu64
                        " bytes\n"
                        "- Binding stride: %" PRIu64
                        " bytes\n"
                        "- Vertices count: %" PRIu64
                        "\n"

                        "At least the following vertex attribute caused OOB access:\n"
                        "- location: %" PRIu32
                        "\n"
                        "- binding: %" PRIu32
                        "\n"
                        "- format: %s\n"
                        "- offset: %" PRIu32
                        " bytes\n"

                        "Index buffer binding info:\n"
                        "- Buffer: %s\n"
                        "- Index type: %s\n"
                        "- Binding offset: %" PRIu64
                        " bytes\n"
                        "- Binding size: %" PRIu64 " bytes (or %" PRIu32
                        " %s)\n"

                        "Note: Vertex buffer binding size is the effective, valid one, based on how the VkBuffer was created and "
                        "vertex buffer binding parameters. So it can be clamped up to 0 if binding was invalid.\n",
                        // OOB vertex index info
                        vertex_index, index_buffer_offset, index_buffer_value, vertex_offset, vertex_index,

                        // Vertex buffer binding info
                        gpuav.FormatHandle(smallest_vertex_buffer_binding.binding_info.buffer).c_str(),
                        smallest_vertex_buffer_binding.binding, smallest_vertex_buffer_binding.binding_info.size,
                        smallest_vertex_buffer_binding.binding_info.offset, smallest_vertex_buffer_binding.binding_info.stride,
                        smallest_vertex_buffer_binding.smallest_vertex_attributes_count,

                        // Vertex Attribute info
                        smallest_vertex_buffer_binding.attribute.location, smallest_vertex_buffer_binding.attribute.binding,
                        string_VkFormat(smallest_vertex_buffer_binding.attribute.format),
                        smallest_vertex_buffer_binding.attribute.offset,

                        // Index buffer binding info
                        gpuav.FormatHandle(index_buffer_binding.buffer).c_str(),
                        string_VkIndexType(index_buffer_binding.index_type), index_buffer_binding.offset, index_buffer_binding.size,
                        max_indices_in_buffer, string_VkIndexType(index_buffer_binding.index_type));
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
    static size_t GetSpirvSize() { return cmd_validation_draw_indexed_indirect_index_buffer_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return cmd_validation_draw_indexed_indirect_index_buffer_comp; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::DrawIndexedIndirectIndexBufferPushData push_constants{};
    BoundStorageBuffer draw_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndirectBuffer};
    BoundStorageBuffer count_buffer_binding = {gpuav::glsl::kPreDrawBinding_CountBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_IndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},
            {gpuav::glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
            {gpuav::glsl::kPreDrawBinding_IndexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) const {
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

void DrawIndexedIndirectIndexBuffer(Validator &gpuav, CommandBuffer &cb_state, const Location &loc, VkBuffer draw_buffer,
                                    VkDeviceSize draw_buffer_offset, uint32_t draw_cmds_byte_stride, uint32_t draw_count,
                                    VkBuffer count_buffer, VkDeviceSize count_buffer_offset, const char *vuid_oob_index) {
    if (!gpuav.gpuav_settings.validate_index_buffers) {
        return;
    }

    if (gpuav.enabled_features.robustBufferAccess2) {
        const LvlBindPoint lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
        const LastBound &last_bound = cb_state.lastBound[lv_bind_point];
        const vvl::Pipeline *pipeline_state = last_bound.pipeline_state;
        if (pipeline_state) {
            const auto robustness_ci =
                vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfo>(pipeline_state->GraphicsCreateInfo().pNext);
            if (robustness_ci && robustness_ci->vertexInputs) {
                return;
            }
        }
    }

    if (!cb_state.IsPrimary()) {
        // TODO Unhandled for now. Potential issues with accessing the right vertex buffers
        // in secondary command buffers
        return;
    }

    if (!cb_state.index_buffer_binding.buffer) {
        return;
    }

    CommandBuffer::ValidationCommandFunc validation_cmd = [index_buffer_binding = cb_state.index_buffer_binding, draw_buffer,
                                                           draw_buffer_offset, draw_cmds_byte_stride, draw_count, count_buffer,
                                                           count_buffer_offset, draw_i = cb_state.draw_index,
                                                           error_logger_i = uint32_t(cb_state.per_command_error_loggers.size()),
                                                           loc](Validator &gpuav, CommandBuffer &cb_state) {
        SharedDrawValidationResources &shared_draw_validation_resources =
            gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
        if (!shared_draw_validation_resources.valid) return;
        ComputeValidationPipeline<DrawIndexedIndirectIndexBufferShader> &validation_pipeline =
            gpuav.shared_resources_manager.Get<ComputeValidationPipeline<DrawIndexedIndirectIndexBufferShader>>(
                gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
        if (!validation_pipeline.valid) return;

        const uint32_t index_bits_size = GetIndexBitsSize(index_buffer_binding.index_type);
        const uint32_t max_indices_in_buffer = static_cast<uint32_t>(index_buffer_binding.size / (index_bits_size / 8u));
        {
            DrawIndexedIndirectIndexBufferShader shader_resources;
            if (count_buffer != VK_NULL_HANDLE) {
                shader_resources.push_constants.flags |= glsl::kIndexedIndirectDrawFlags_DrawCountFromBuffer;
                shader_resources.count_buffer_binding.info = {count_buffer, 0, count_buffer_offset + sizeof(uint32_t)};
                shader_resources.push_constants.count_buffer_dwords_offset = (uint32_t)count_buffer_offset / sizeof(uint32_t);

            } else {
                shader_resources.count_buffer_binding.info = {shared_draw_validation_resources.dummy_buffer.VkHandle(), 0,
                                                              VK_WHOLE_SIZE};
            }

            shader_resources.push_constants.draw_cmds_stride_dwords = draw_cmds_byte_stride / sizeof(uint32_t);
            shader_resources.push_constants.bound_index_buffer_indices_count = max_indices_in_buffer;
            shader_resources.push_constants.cpu_draw_count = draw_count;

            shader_resources.draw_buffer_binding.info = {draw_buffer, 0, VK_WHOLE_SIZE};
            shader_resources.push_constants.draw_indexed_indirect_cmds_buffer_dwords_offset =
                (uint32_t)draw_buffer_offset / sizeof(uint32_t);

            validation_pipeline.BindShaderResources(gpuav, cb_state, draw_i, error_logger_i, shader_resources);
        }

        {
            DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

            // One draw will check all VkDrawIndexedIndirectCommand
            DispatchCmdDispatch(cb_state.VkHandle(), 1, 1, 1);
            // synchronize draw buffer validation (read) against subsequent writes
            std::array<VkBufferMemoryBarrier, 2> buffer_memory_barriers = {};
            uint32_t buffer_memory_barriers_count = 1;
            buffer_memory_barriers[0] = vku::InitStructHelper();
            buffer_memory_barriers[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            buffer_memory_barriers[0].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            buffer_memory_barriers[0].buffer = draw_buffer;
            buffer_memory_barriers[0].offset = draw_buffer_offset;
            buffer_memory_barriers[0].size = VK_WHOLE_SIZE;

            if (count_buffer) {
                buffer_memory_barriers[1] = vku::InitStructHelper();
                buffer_memory_barriers[1].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                buffer_memory_barriers[1].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                buffer_memory_barriers[1].buffer = count_buffer;
                buffer_memory_barriers[1].offset = count_buffer_offset;
                buffer_memory_barriers[1].size = sizeof(uint32_t);
                ++buffer_memory_barriers_count;
            }

            DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, buffer_memory_barriers_count,
                                       buffer_memory_barriers.data(), 0, nullptr);
        }
    };
    cb_state.per_render_pass_validation_commands.emplace_back(std::move(validation_cmd));

    CommandBuffer::ErrorLoggerFunc error_logger = [loc, vuid_oob_index, draw_buffer, draw_buffer_offset, draw_cmds_byte_stride,
                                                   index_buffer_binding = cb_state.index_buffer_binding](
                                                      Validator &gpuav, const CommandBuffer &, const uint32_t *error_record,
                                                      const LogObjectList &objlist,
                                                      const std::vector<std::string> &initial_label_stack) {
        bool skip = false;

        using namespace glsl;

        switch (error_record[kHeaderErrorSubCodeOffset]) {
            case kErrorSubCode_OobIndexBuffer: {
                const uint32_t draw_i = error_record[kPreActionParamOffset_0];
                const uint32_t first_index = error_record[kPreActionParamOffset_1];
                const uint32_t index_count = error_record[kPreActionParamOffset_2];
                const uint32_t highest_accessed_index = first_index + index_count;
                const uint32_t index_bits_size = GetIndexBitsSize(index_buffer_binding.index_type);
                const uint32_t max_indices_in_buffer = static_cast<uint32_t>(index_buffer_binding.size / (index_bits_size / 8u));

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
    static size_t GetSpirvSize() { return cmd_validation_draw_indexed_indirect_vertex_buffer_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return cmd_validation_draw_indexed_indirect_vertex_buffer_comp; }

    static const uint32_t desc_set_id = gpuav::glsl::kDiagPerCmdDescriptorSet;

    glsl::DrawIndexedIndirectVertexBufferPushData push_constants{};
    BoundStorageBuffer draw_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndirectBuffer};
    BoundStorageBuffer count_buffer_binding = {gpuav::glsl::kPreDrawBinding_CountBuffer};
    BoundStorageBuffer index_buffer_binding = {gpuav::glsl::kPreDrawBinding_IndexBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {gpuav::glsl::kPreDrawBinding_IndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},
            {gpuav::glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
            {gpuav::glsl::kPreDrawBinding_IndexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) const {
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
    if (!gpuav.gpuav_settings.validate_index_buffers) {
        return;
    }

    if (gpuav.enabled_features.robustBufferAccess) {
        return;
    }

    if (!cb_state.IsPrimary()) {
        // TODO Unhandled for now. Potential issues with accessing the right vertex buffers
        // in secondary command buffers
        return;
    }

    if (!cb_state.index_buffer_binding.buffer) {
        return;
    }

    const std::optional<SmallestVertexBufferBinding> smallest_vertex_buffer_binding = SmallestVertexAttributesCount(cb_state);
    if (!smallest_vertex_buffer_binding.has_value()) {
        return;
    }

    CommandBuffer::ValidationCommandFunc validation_cmd =
        [index_buffer_binding = cb_state.index_buffer_binding, smallest_vertex_buffer_binding = *smallest_vertex_buffer_binding,
         draw_buffer, draw_buffer_offset, draw_cmds_byte_stride, draw_count, count_buffer, count_buffer_offset,
         draw_i = cb_state.draw_index, error_logger_i = uint32_t(cb_state.per_command_error_loggers.size()),
         loc](Validator &gpuav, CommandBuffer &cb_state) {
            SharedDrawValidationResources &shared_draw_validation_resources =
                gpuav.shared_resources_manager.Get<SharedDrawValidationResources>(gpuav, loc);
            if (!shared_draw_validation_resources.valid) return;
            ComputeValidationPipeline<DrawIndexedIndirectVertexBufferShader> &validation_pipeline =
                gpuav.shared_resources_manager.Get<ComputeValidationPipeline<DrawIndexedIndirectVertexBufferShader>>(
                    gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
            if (!validation_pipeline.valid) return;

            const uint32_t index_bits_size = GetIndexBitsSize(index_buffer_binding.index_type);
            const uint32_t max_indices_in_buffer = static_cast<uint32_t>(index_buffer_binding.size / (index_bits_size / 8u));

            {
                DrawIndexedIndirectVertexBufferShader shader_resources;
                if (count_buffer != VK_NULL_HANDLE) {
                    shader_resources.push_constants.flags |= glsl::kIndexedIndirectDrawFlags_DrawCountFromBuffer;
                    shader_resources.count_buffer_binding.info = {count_buffer, 0, count_buffer_offset + sizeof(uint32_t)};
                    shader_resources.push_constants.count_buffer_dwords_offset = (uint32_t)count_buffer_offset / sizeof(uint32_t);
                } else {
                    shader_resources.count_buffer_binding.info = {shared_draw_validation_resources.dummy_buffer.VkHandle(), 0,
                                                                  VK_WHOLE_SIZE};
                }

                shader_resources.push_constants.index_width = index_bits_size;
                shader_resources.push_constants.draw_cmds_stride_dwords = draw_cmds_byte_stride / sizeof(uint32_t);
                shader_resources.push_constants.bound_index_buffer_indices_count = max_indices_in_buffer;
                shader_resources.push_constants.cpu_draw_count = draw_count;

                shader_resources.push_constants.smallest_vertex_attributes_count =
                    static_cast<uint32_t>(smallest_vertex_buffer_binding.smallest_vertex_attributes_count);

                shader_resources.draw_buffer_binding.info = {draw_buffer, 0, VK_WHOLE_SIZE};
                shader_resources.push_constants.draw_indexed_indirect_cmds_buffer_dwords_offset =
                    (uint32_t)draw_buffer_offset / sizeof(uint32_t);
                shader_resources.index_buffer_binding.info = {index_buffer_binding.buffer, 0,
                                                              index_buffer_binding.offset + index_buffer_binding.size};
                shader_resources.push_constants.index_buffer_dwords_offset =
                    (uint32_t)index_buffer_binding.offset / sizeof(uint32_t);

                validation_pipeline.BindShaderResources(gpuav, cb_state, draw_i, error_logger_i, shader_resources);
            }

            {
                DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

                // Dispatch as many draws as there are indices in index buffer.
                // Each shader invocation will inspect one index, for all draw commands.
                // Shader is in charge of performing validation only for indices within the supplied ranges.
                constexpr uint32_t work_group_size = 64;
                const uint32_t indices_to_check = max_indices_in_buffer;
                const uint32_t work_group_count =
                    indices_to_check / work_group_size + uint32_t(indices_to_check % work_group_size != 0);
                VVL_TracyPlot("gpuav::valcmd::DrawIndexedIndirectVertexBuffer Dispatch size", int64_t(work_group_count));
                DispatchCmdDispatch(cb_state.VkHandle(), work_group_count, 1, 1);
                // synchronize draw buffer validation (read) against subsequent writes
                std::array<VkBufferMemoryBarrier, 3> buffer_memory_barriers = {};
                uint32_t buffer_memory_barriers_count = 2;
                buffer_memory_barriers[0] = vku::InitStructHelper();
                buffer_memory_barriers[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                buffer_memory_barriers[0].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                buffer_memory_barriers[0].buffer = draw_buffer;
                buffer_memory_barriers[0].offset = draw_buffer_offset;
                buffer_memory_barriers[0].size = VK_WHOLE_SIZE;

                buffer_memory_barriers[1] = vku::InitStructHelper();
                buffer_memory_barriers[1].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                buffer_memory_barriers[1].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                buffer_memory_barriers[1].buffer = index_buffer_binding.buffer;
                buffer_memory_barriers[1].offset = index_buffer_binding.offset;
                buffer_memory_barriers[1].size = index_buffer_binding.size;

                if (count_buffer) {
                    buffer_memory_barriers[2] = vku::InitStructHelper();
                    buffer_memory_barriers[2].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    buffer_memory_barriers[2].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                    buffer_memory_barriers[2].buffer = count_buffer;
                    buffer_memory_barriers[2].offset = count_buffer_offset;
                    buffer_memory_barriers[2].size = sizeof(uint32_t);
                    ++buffer_memory_barriers_count;
                }

                DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, buffer_memory_barriers_count,
                                           buffer_memory_barriers.data(), 0, nullptr);
            }
        };
    cb_state.per_render_pass_validation_commands.emplace_back(std::move(validation_cmd));

    CommandBuffer::ErrorLoggerFunc error_logger =
        [loc, vuid_oob_vertex, draw_buffer, smallest_vertex_buffer_binding = *smallest_vertex_buffer_binding,
         index_buffer_binding = cb_state.index_buffer_binding](Validator &gpuav, const CommandBuffer &,
                                                               const uint32_t *error_record, const LogObjectList &objlist,
                                                               const std::vector<std::string> &initial_label_stack) {
            bool skip = false;

            using namespace glsl;

            switch (error_record[kHeaderErrorSubCodeOffset]) {
                case kErrorSubCode_OobVertexBuffer: {
                    const uint32_t draw_i = error_record[kPreActionParamOffset_0];
                    const uint32_t index_buffer_offset = error_record[kPreActionParamOffset_1];
                    const int32_t vertex_offset = static_cast<int32_t>(error_record[kPreActionParamOffset_2]);
                    const uint32_t vertex_index = error_record[kPreActionParamOffset_3];
                    const uint32_t index_buffer_value = static_cast<uint32_t>(int32_t(vertex_index) - vertex_offset);
                    const uint32_t index_bits_size = GetIndexBitsSize(index_buffer_binding.index_type);
                    const uint32_t max_indices_in_buffer =
                        static_cast<uint32_t>(index_buffer_binding.size / (index_bits_size / 8u));

                    gpuav.LogError(
                        vuid_oob_vertex, objlist, loc,
                        "Vertex index %" PRIu32
                        " is not within the smallest bound vertex buffer. Computed from VkDrawIndexedIndirectCommand[ %" PRIu32
                        " ], stored in %s.\n"
                        "index_buffer[ %" PRIu32 " ] (%" PRIu32 ") + vertexOffset (%" PRIi32 ") = Vertex index %" PRIu32
                        "\n"

                        "Smallest vertex buffer binding info, causing OOB access:\n"
                        "- Buffer: %s\n"
                        "- Binding: %" PRIu32
                        "\n"
                        "- Binding size (effective): %" PRIu64
                        " bytes\n"
                        "- Binding offset: %" PRIu64
                        " bytes\n"
                        "- Binding stride: %" PRIu64
                        " bytes\n"
                        "- Vertices count: %" PRIu64
                        "\n"

                        "At least the following vertex attribute caused OOB access:\n"
                        "- location: %" PRIu32
                        "\n"
                        "- binding: %" PRIu32
                        "\n"
                        "- format: %s\n"
                        "- offset: %" PRIu32
                        " bytes\n"

                        "Index buffer binding info:\n"
                        "- Buffer: %s\n"
                        "- Index type: %s\n"
                        "- Binding offset: %" PRIu64
                        " bytes\n"
                        "- Binding size: %" PRIu64 " bytes (or %" PRIu32
                        " %s)\n"

                        "Note: Vertex buffer binding size is the effective, valid one, based on how the VkBuffer was created and "
                        "vertex buffer binding parameters. So it can be clamped up to 0 if binding was invalid.\n",
                        // OOB vertex index info
                        vertex_index, draw_i, gpuav.FormatHandle(draw_buffer).c_str(), index_buffer_offset, index_buffer_value,
                        vertex_offset, vertex_index,

                        // Vertex buffer binding info
                        gpuav.FormatHandle(smallest_vertex_buffer_binding.binding_info.buffer).c_str(),
                        smallest_vertex_buffer_binding.binding, smallest_vertex_buffer_binding.binding_info.size,
                        smallest_vertex_buffer_binding.binding_info.offset, smallest_vertex_buffer_binding.binding_info.stride,
                        smallest_vertex_buffer_binding.smallest_vertex_attributes_count,

                        // Vertex Attribute info
                        smallest_vertex_buffer_binding.attribute.location, smallest_vertex_buffer_binding.attribute.binding,
                        string_VkFormat(smallest_vertex_buffer_binding.attribute.format),
                        smallest_vertex_buffer_binding.attribute.offset,

                        // Index buffer binding info
                        gpuav.FormatHandle(index_buffer_binding.buffer).c_str(),
                        string_VkIndexType(index_buffer_binding.index_type), index_buffer_binding.offset, index_buffer_binding.size,
                        max_indices_in_buffer, string_VkIndexType(index_buffer_binding.index_type));

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

}  // namespace valcmd
}  // namespace gpuav
