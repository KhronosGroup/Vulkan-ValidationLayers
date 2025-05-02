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

#include "gpuav/validation_cmd/gpuav_draw.h"

#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_validation_pipeline.h"
#include "gpuav/validation_cmd/gpuav_validation_cmd_common.h"
#include "gpuav/error_message/gpuav_vuids.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"

#include "state_tracker/render_pass_state.h"
#include "state_tracker/pipeline_state.h"
#include "containers/limits.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/shaders/validation_cmd/push_data.h"
#include "generated/gpuav_offline_spirv.h"

#include "profiling/profiling.h"

namespace gpuav {
namespace valcmd {

using ValidationCommandFunc = CommandBufferSubState::ValidationCommandFunc;
using ErrorLoggerFunc = CommandBufferSubState::ErrorLoggerFunc;

struct SharedDrawValidationResources {
    vko::Buffer dummy_buffer;  // Used to fill unused buffer bindings in validation pipelines
    bool valid = false;

    SharedDrawValidationResources(Validator &gpuav) : dummy_buffer(gpuav) {
        VkBufferCreateInfo dummy_buffer_info = vku::InitStructHelper();
        dummy_buffer_info.size = 64;// whatever
        dummy_buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
        const bool success = dummy_buffer.Create(&dummy_buffer_info, &alloc_info);
        if (!success) {
            valid = false;
            return;
        }

        valid = true;
    }

    ~SharedDrawValidationResources() { dummy_buffer.Destroy(); }
};

void FlushValidationCmds(Validator &gpuav, CommandBufferSubState &cb_state) {
    valpipe::RestorablePipelineState restorable_state(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    for (auto &validation_cmd : cb_state.per_render_pass_validation_commands) {
        validation_cmd(gpuav, cb_state);
    }
    cb_state.per_render_pass_validation_commands.clear();
}

struct FirstInstanceValidationShader {
    static size_t GetSpirvSize() { return validation_cmd_first_instance_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return validation_cmd_first_instance_comp; }

    glsl::FirstInstancePushData push_constants{};
    valpipe::BoundStorageBuffer draw_buffer_binding = {glsl::kPreDrawBinding_IndirectBuffer};
    valpipe::BoundStorageBuffer count_buffer_binding = {glsl::kPreDrawBinding_CountBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {glsl::kPreDrawBinding_IndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},  // indirect buffer
            {glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
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

// Use "api_" prefix to make it clear which buffer/offset/etc we are talking about
// "api" helps to distinguish it is input from the user at the API level
void FirstInstance(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc, VkBuffer api_buffer,
                   VkDeviceSize api_offset, uint32_t api_stride, vvl::Struct api_struct_name, uint32_t first_instance_member_pos,
                   uint32_t api_draw_count, VkBuffer api_count_buffer, VkDeviceSize api_count_buffer_offset, const char *vuid) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    if (gpuav.enabled_features.drawIndirectFirstInstance) {
        return;
    }

    if (cb_state.max_actions_cmd_validation_reached_) {
        return;
    }

    ValidationCommandFunc validation_cmd = [api_buffer, api_offset, api_stride, first_instance_member_pos, api_draw_count,
                                            api_count_buffer, api_count_buffer_offset, draw_i = cb_state.draw_index,
                                            error_logger_i = uint32_t(cb_state.per_command_error_loggers.size()),
                                            loc](Validator &gpuav, CommandBufferSubState &cb_state) {
        SharedDrawValidationResources &shared_draw_validation_resources =
            gpuav.shared_resources_manager.GetOrCreate<SharedDrawValidationResources>(gpuav);
        if (!shared_draw_validation_resources.valid) return;
        valpipe::ComputePipeline<FirstInstanceValidationShader> &validation_pipeline =
            gpuav.shared_resources_manager.GetOrCreate<valpipe::ComputePipeline<FirstInstanceValidationShader>>(
                gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
        if (!validation_pipeline.valid) return;

        auto draw_buffer_state = gpuav.Get<vvl::Buffer>(api_buffer);
        if (!draw_buffer_state) {
            gpuav.InternalError(LogObjectList(cb_state.VkHandle(), api_buffer), loc, "buffer must be a valid VkBuffer handle");
            return;
        }

        // Setup shader resources
        // ---
        {
            FirstInstanceValidationShader shader_resources;
            shader_resources.push_constants.api_stride_dwords = api_stride / sizeof(uint32_t);
            shader_resources.push_constants.api_draw_count = api_draw_count;
            shader_resources.push_constants.first_instance_member_pos = first_instance_member_pos;

            shader_resources.draw_buffer_binding.info = {api_buffer, 0, VK_WHOLE_SIZE};
            shader_resources.push_constants.api_offset_dwords = (uint32_t)api_offset / sizeof(uint32_t);
            if (api_count_buffer) {
                shader_resources.push_constants.flags |= glsl::kFirstInstanceFlags_DrawCountFromBuffer;
                shader_resources.count_buffer_binding.info = {api_count_buffer, 0, sizeof(uint32_t)};
                shader_resources.push_constants.api_count_buffer_offset_dwords =
                    uint32_t(api_count_buffer_offset / sizeof(uint32_t));

            } else {
                shader_resources.count_buffer_binding.info = {shared_draw_validation_resources.dummy_buffer.VkHandle(), 0,
                                                              VK_WHOLE_SIZE};
            }

            if (!BindShaderResources(validation_pipeline, gpuav, cb_state, draw_i, error_logger_i, shader_resources)) {
                return;
            }
        }

        // Setup validation pipeline
        // ---
        {
            DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

            uint32_t max_held_draw_cmds = 0;
            if (draw_buffer_state->create_info.size > api_offset) {
                // If drawCount is less than or equal to one, stride is ignored
                if (api_draw_count > 1) {
                    max_held_draw_cmds = static_cast<uint32_t>((draw_buffer_state->create_info.size - api_offset) / api_stride);
                } else {
                    max_held_draw_cmds = 1;
                }
            }
            // It is assumed that the number of draws to validate is fairly low.
            // Otherwise might reconsider having a warp dimension of (1, 1, 1)
            // Maybe another reason to add telemetry?
            const uint32_t work_group_count = std::min(api_draw_count, max_held_draw_cmds);

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
            buffer_memory_barriers[0].buffer = api_buffer;
            buffer_memory_barriers[0].offset = api_offset;
            buffer_memory_barriers[0].size = work_group_count * sizeof(uint32_t);

            if (api_count_buffer) {
                buffer_memory_barriers[1] = vku::InitStructHelper();
                buffer_memory_barriers[1].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                buffer_memory_barriers[1].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                buffer_memory_barriers[1].buffer = api_count_buffer;
                buffer_memory_barriers[1].offset = api_count_buffer_offset;
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
    const uint32_t label_command_i =
        !cb_state.base.GetLabelCommands().empty() ? uint32_t(cb_state.base.GetLabelCommands().size() - 1) : vvl::kU32Max;
    ErrorLoggerFunc error_logger = [&gpuav, &cb_state, loc, vuid, api_struct_name, label_command_i](
                                       const uint32_t *error_record, const LogObjectList &objlist,
                                       const std::vector<std::string> &initial_label_stack) {
        bool skip = false;
        using namespace glsl;

        const uint32_t error_group = error_record[kHeaderShaderIdErrorOffset] >> kErrorGroupShift;
        if (error_group != kErrorGroupGpuPreDraw) {
            assert(false);
            return skip;
        }

        assert(((error_record[kHeaderShaderIdErrorOffset] & kErrorSubCodeMask) >> kErrorSubCodeShift) ==
               kErrorSubCodePreDrawFirstInstance);

        const uint32_t index = error_record[kPreActionParamOffset_0];
        const uint32_t invalid_first_instance = error_record[kPreActionParamOffset_1];

        std::string debug_region_name = cb_state.GetDebugLabelRegion(label_command_i, initial_label_stack);
        Location loc_with_debug_region(loc, debug_region_name);
        skip |= gpuav.LogError(
            vuid, objlist, loc_with_debug_region,
            "The drawIndirectFirstInstance feature is not enabled, but the firstInstance member of the %s structure at "
            "index %" PRIu32 " is %" PRIu32 ".",
            vvl::String(api_struct_name), index, invalid_first_instance);

        return skip;
    };

    cb_state.per_command_error_loggers.emplace_back(std::move(error_logger));
}

template <>
void FirstInstance<VkDrawIndirectCommand>(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc, VkBuffer buffer,
                                          VkDeviceSize offset, uint32_t draw_count, VkBuffer count_buffer,
                                          VkDeviceSize count_buffer_offset, const char *vuid) {
    FirstInstance(gpuav, cb_state, loc, buffer, offset, sizeof(VkDrawIndirectCommand), vvl::Struct::VkDrawIndirectCommand, 3,
                  draw_count, count_buffer, count_buffer_offset, vuid);
}

template <>
void FirstInstance<VkDrawIndexedIndirectCommand>(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc,
                                                 VkBuffer buffer, VkDeviceSize offset, uint32_t draw_count, VkBuffer count_buffer,
                                                 VkDeviceSize count_buffer_offset, const char *vuid) {
    FirstInstance(gpuav, cb_state, loc, buffer, offset, sizeof(VkDrawIndexedIndirectCommand),
                  vvl::Struct::VkDrawIndexedIndirectCommand, 4, draw_count, count_buffer, count_buffer_offset, vuid);
}

struct CountBufferValidationShader {
    static size_t GetSpirvSize() { return validation_cmd_count_buffer_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return validation_cmd_count_buffer_comp; }

    glsl::CountBufferPushData push_constants{};
    valpipe::BoundStorageBuffer count_buffer_binding = {glsl::kPreDrawBinding_CountBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
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

void CountBuffer(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc, VkBuffer api_buffer,
                 VkDeviceSize api_offset, uint32_t api_struct_size_byte, vvl::Struct api_struct_name, uint32_t api_stride,
                 VkBuffer api_count_buffer, VkDeviceSize api_count_buffer_offset, const char *vuid) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    if (!gpuav.modified_features.shaderInt64) {
        return;
    }

    if (cb_state.max_actions_cmd_validation_reached_) {
        return;
    }

    auto draw_buffer_state = gpuav.Get<vvl::Buffer>(api_buffer);
    if (!draw_buffer_state) {
        gpuav.InternalError(LogObjectList(cb_state.VkHandle(), api_buffer), loc, "buffer must be a valid VkBuffer handle");
        return;
    }

    ValidationCommandFunc validation_cmd = [draw_buffer_size = draw_buffer_state->create_info.size, api_offset,
                                            api_struct_size_byte, api_stride, api_count_buffer, api_count_buffer_offset,
                                            draw_i = cb_state.draw_index,
                                            error_logger_i = uint32_t(cb_state.per_command_error_loggers.size()),
                                            loc](Validator &gpuav, CommandBufferSubState &cb_state) {
        SharedDrawValidationResources &shared_draw_validation_resources =
            gpuav.shared_resources_manager.GetOrCreate<SharedDrawValidationResources>(gpuav);
        if (!shared_draw_validation_resources.valid) return;
        valpipe::ComputePipeline<CountBufferValidationShader> &validation_pipeline =
            gpuav.shared_resources_manager.GetOrCreate<valpipe::ComputePipeline<CountBufferValidationShader>>(
                gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
        if (!validation_pipeline.valid) return;

        // Setup shader resources
        // ---
        {
            CountBufferValidationShader shader_resources;
            shader_resources.push_constants.api_stride = api_stride;
            shader_resources.push_constants.api_offset = api_offset;
            shader_resources.push_constants.draw_buffer_size = draw_buffer_size;
            shader_resources.push_constants.api_struct_size_byte = api_struct_size_byte;
            shader_resources.push_constants.device_limit_max_draw_indirect_count = gpuav.phys_dev_props.limits.maxDrawIndirectCount;

            shader_resources.count_buffer_binding.info = {api_count_buffer, 0, sizeof(uint32_t)};
            shader_resources.push_constants.api_count_buffer_offset_dwords = uint32_t(api_count_buffer_offset / sizeof(uint32_t));

            if (!BindShaderResources(validation_pipeline, gpuav, cb_state, draw_i, error_logger_i, shader_resources)) {
                return;
            }
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
            count_buffer_memory_barrier.buffer = api_count_buffer;
            count_buffer_memory_barrier.offset = api_count_buffer_offset;
            count_buffer_memory_barrier.size = sizeof(uint32_t);

            DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &count_buffer_memory_barrier, 0,
                                       nullptr);
        }
    };

    cb_state.per_render_pass_validation_commands.emplace_back(std::move(validation_cmd));

    // Register error logger
    // ---
    const uint32_t label_command_i =
        !cb_state.base.GetLabelCommands().empty() ? uint32_t(cb_state.base.GetLabelCommands().size() - 1) : vvl::kU32Max;
    ErrorLoggerFunc error_logger = [&gpuav, &cb_state, loc, api_buffer, draw_buffer_size = draw_buffer_state->create_info.size,
                                    api_offset, api_struct_size_byte, api_stride, api_struct_name, vuid,
                                    label_command_i](const uint32_t *error_record, const LogObjectList &objlist,
                                                     const std::vector<std::string> &initial_label_stack) {
        bool skip = false;
        using namespace glsl;

        std::string debug_region_name = cb_state.GetDebugLabelRegion(label_command_i, initial_label_stack);
        Location loc_with_debug_region(loc, debug_region_name);

        const uint32_t error_sub_code = (error_record[kHeaderShaderIdErrorOffset] & kErrorSubCodeMask) >> kErrorSubCodeShift;
        switch (error_sub_code) {
            case kErrorSubCodePreDraw_DrawBufferSize: {
                const uint32_t count = error_record[kPreActionParamOffset_0];

                const VkDeviceSize draw_size = (api_stride * (count - 1) + api_offset + api_struct_size_byte);

                // Discussed that if drawCount is largeer than the buffer, it is still capped by the maxDrawCount on the CPU (which
                // we would have checked is in the buffer range). We decided that we still want to give a warning, but the nothing
                // is invalid here. https://gitlab.khronos.org/vulkan/vulkan/-/issues/3991
                skip |= gpuav.LogWarning("WARNING-GPU-AV-drawCount", objlist, loc_with_debug_region,
                                         "Indirect draw count of %" PRIu32 " would exceed size (%" PRIu64
                                         ") of buffer (%s). "
                                         "stride = %" PRIu32 " offset = %" PRIu64
                                         " (stride * (drawCount - 1) + offset + sizeof(%s)) = %" PRIu64 ".",
                                         count, draw_buffer_size, gpuav.FormatHandle(api_buffer).c_str(), api_stride, api_offset,
                                         vvl::String(api_struct_name), draw_size);
                break;
            }
            case kErrorSubCodePreDraw_DrawCountLimit: {
                const uint32_t count = error_record[kPreActionParamOffset_0];
                skip |= gpuav.LogError(vuid, objlist, loc_with_debug_region,
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
    static size_t GetSpirvSize() { return validation_cmd_draw_mesh_indirect_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return validation_cmd_draw_mesh_indirect_comp; }

    glsl::DrawMeshPushData push_constants{};
    valpipe::BoundStorageBuffer draw_buffer_binding = {glsl::kPreDrawBinding_IndirectBuffer};
    valpipe::BoundStorageBuffer count_buffer_binding = {glsl::kPreDrawBinding_CountBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {glsl::kPreDrawBinding_IndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},  // indirect buffer
            {glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
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

void DrawMeshIndirect(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc, VkBuffer api_buffer,
                      VkDeviceSize api_offset, uint32_t api_stride, VkBuffer api_count_buffer, VkDeviceSize api_count_buffer_offset,
                      uint32_t api_draw_count) {
    if (!gpuav.gpuav_settings.validate_indirect_draws_buffers) {
        return;
    }

    if (cb_state.max_actions_cmd_validation_reached_) {
        return;
    }

    auto draw_buffer_state = gpuav.Get<vvl::Buffer>(api_buffer);
    if (!draw_buffer_state) {
        gpuav.InternalError(LogObjectList(cb_state.VkHandle(), api_buffer), loc, "buffer must be a valid VkBuffer handle");
        return;
    }

    const LvlBindPoint lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const LastBound &last_bound = cb_state.base.lastBound[lv_bind_point];
    const vvl::Pipeline *pipeline_state = last_bound.pipeline_state;
    const VkShaderStageFlags stages = pipeline_state->create_info_shaders;
    const bool is_task_shader = (stages & VK_SHADER_STAGE_TASK_BIT_EXT) == VK_SHADER_STAGE_TASK_BIT_EXT;

    ValidationCommandFunc validation_cmd =
        [api_buffer, draw_buffer_full_size = draw_buffer_state->create_info.size, api_offset, api_stride, api_count_buffer,
         api_count_buffer_offset, api_draw_count, is_task_shader, draw_i = cb_state.draw_index,
         error_logger_i = uint32_t(cb_state.per_command_error_loggers.size()),
         loc](Validator &gpuav, CommandBufferSubState &cb_state) {
            SharedDrawValidationResources &shared_draw_validation_resources =
                gpuav.shared_resources_manager.GetOrCreate<SharedDrawValidationResources>(gpuav);
            if (!shared_draw_validation_resources.valid) return;
            valpipe::ComputePipeline<MeshValidationShader> &validation_pipeline =
                gpuav.shared_resources_manager.GetOrCreate<valpipe::ComputePipeline<MeshValidationShader>>(
                    gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
            if (!validation_pipeline.valid) return;

            // Setup shader resources
            // ---
            {
                MeshValidationShader shader_resources;
                shader_resources.push_constants.api_stride_dwords = api_stride / sizeof(uint32_t);
                shader_resources.push_constants.api_draw_count = api_draw_count;
                const auto &properties = gpuav.phys_dev_ext_props.mesh_shader_props_ext;
                if (is_task_shader) {
                    shader_resources.push_constants.max_workgroup_count_x = properties.maxTaskWorkGroupCount[0];
                    shader_resources.push_constants.max_workgroup_count_y = properties.maxTaskWorkGroupCount[1];
                    shader_resources.push_constants.max_workgroup_count_z = properties.maxTaskWorkGroupCount[2];
                    shader_resources.push_constants.max_workgroup_total_count = properties.maxTaskWorkGroupTotalCount;
                } else {
                    shader_resources.push_constants.max_workgroup_count_x = properties.maxMeshWorkGroupCount[0];
                    shader_resources.push_constants.max_workgroup_count_y = properties.maxMeshWorkGroupCount[1];
                    shader_resources.push_constants.max_workgroup_count_z = properties.maxMeshWorkGroupCount[2];
                    shader_resources.push_constants.max_workgroup_total_count = properties.maxMeshWorkGroupTotalCount;
                }

                shader_resources.draw_buffer_binding.info = {api_buffer, 0, VK_WHOLE_SIZE};
                shader_resources.push_constants.api_offset_dwords = uint32_t(api_offset / sizeof(uint32_t));
                if (api_count_buffer != VK_NULL_HANDLE) {
                    shader_resources.push_constants.flags |= glsl::kDrawMeshFlags_DrawCountFromBuffer;
                    shader_resources.count_buffer_binding.info = {api_count_buffer, 0, sizeof(uint32_t)};
                    shader_resources.push_constants.api_count_buffer_offset_dwords =
                        uint32_t(api_count_buffer_offset / sizeof(uint32_t));
                } else {
                    shader_resources.count_buffer_binding.info = {shared_draw_validation_resources.dummy_buffer.VkHandle(), 0,
                                                                  VK_WHOLE_SIZE};
                }

                if (!BindShaderResources(validation_pipeline, gpuav, cb_state, draw_i, error_logger_i, shader_resources)) {
                    return;
                }
            }

            // Setup validation pipeline
            // ---
            {
                DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

                uint32_t max_held_draw_cmds = 0;
                if (draw_buffer_full_size > api_offset) {
                    // If drawCount is less than or equal to one, stride is ignored
                    if (api_draw_count > 1) {
                        max_held_draw_cmds = static_cast<uint32_t>((draw_buffer_full_size - api_offset) / api_stride);
                    } else {
                        max_held_draw_cmds = 1;
                    }
                }
                const uint32_t work_group_count = std::min(api_draw_count, max_held_draw_cmds);
                VVL_TracyPlot("gpuav::valcmd::DrawMeshIndirect Dispatch size", int64_t(work_group_count));
                DispatchCmdDispatch(cb_state.VkHandle(), work_group_count, 1, 1);

                // synchronize draw buffer validation (read) against subsequent writes
                std::array<VkBufferMemoryBarrier, 2> buffer_memory_barriers = {};
                uint32_t buffer_memory_barriers_count = 1;
                buffer_memory_barriers[0] = vku::InitStructHelper();
                buffer_memory_barriers[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                buffer_memory_barriers[0].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                buffer_memory_barriers[0].buffer = api_buffer;
                buffer_memory_barriers[0].offset = api_offset;
                buffer_memory_barriers[0].size = work_group_count * sizeof(uint32_t);

                if (api_count_buffer) {
                    buffer_memory_barriers[1] = vku::InitStructHelper();
                    buffer_memory_barriers[1].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    buffer_memory_barriers[1].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                    buffer_memory_barriers[1].buffer = api_count_buffer;
                    buffer_memory_barriers[1].offset = api_count_buffer_offset;
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
    const uint32_t label_command_i =
        !cb_state.base.GetLabelCommands().empty() ? uint32_t(cb_state.base.GetLabelCommands().size() - 1) : vvl::kU32Max;
    ErrorLoggerFunc error_logger = [&gpuav, &cb_state, loc, is_task_shader, label_command_i](
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

        std::string debug_region_name = cb_state.GetDebugLabelRegion(label_command_i, initial_label_stack);
        Location loc_with_debug_region(loc, debug_region_name);

        const uint32_t error_sub_code = (error_record[kHeaderShaderIdErrorOffset] & kErrorSubCodeMask) >> kErrorSubCodeShift;
        switch (error_sub_code) {
            case kErrorSubCodePreDrawGroupCountX: {
                const char *vuid_group_count_exceeds_max =
                    is_task_shader ? vuid_task_group_count_exceeds_max_x : vuid_mesh_group_count_exceeds_max_x;
                const uint32_t group_count_x = error_record[kPreActionParamOffset_0];
                const uint32_t limit = is_task_shader ? gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupCount[0]
                                                      : gpuav.phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0];
                skip |= gpuav.LogError(vuid_group_count_exceeds_max, objlist, loc_with_debug_region,
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
                skip |= gpuav.LogError(vuid_group_count_exceeds_max, objlist, loc_with_debug_region,
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
                skip |= gpuav.LogError(vuid_group_count_exceeds_max, objlist, loc_with_debug_region,
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
                skip |= gpuav.LogError(vuid_group_count_exceeds_max, objlist, loc_with_debug_region,
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

struct DrawIndexedIndirectIndexBufferShader {
    static size_t GetSpirvSize() { return validation_cmd_draw_indexed_indirect_index_buffer_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return validation_cmd_draw_indexed_indirect_index_buffer_comp; }

    glsl::DrawIndexedIndirectIndexBufferPushData push_constants{};
    valpipe::BoundStorageBuffer draw_buffer_binding = {glsl::kPreDrawBinding_IndirectBuffer};
    valpipe::BoundStorageBuffer count_buffer_binding = {glsl::kPreDrawBinding_CountBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {glsl::kPreDrawBinding_IndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
            {glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
            {glsl::kPreDrawBinding_IndexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};

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

struct SetupDrawCountDispatchIndirectShader {
    static size_t GetSpirvSize() { return validation_cmd_setup_draw_indexed_indirect_index_buffer_comp_size * sizeof(uint32_t); }
    static const uint32_t *GetSpirv() { return validation_cmd_setup_draw_indexed_indirect_index_buffer_comp; }

    glsl::DrawIndexedIndirectIndexBufferPushData push_constants{};
    valpipe::BoundStorageBuffer count_buffer_binding = {glsl::kPreDrawBinding_CountBuffer};
    valpipe::BoundStorageBuffer dispatch_indirect_buffer_binding = {glsl::kPreDrawBinding_DispatchIndirectBuffer};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {glsl::kPreDrawBinding_CountBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
            {glsl::kPreDrawBinding_DispatchIndirectBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr},
        };

        return bindings;
    }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet desc_set) const {
        std::vector<VkWriteDescriptorSet> desc_writes(2);

        desc_writes[0] = vku::InitStructHelper();
        desc_writes[0].dstSet = desc_set;
        desc_writes[0].dstBinding = count_buffer_binding.binding;
        desc_writes[0].dstArrayElement = 0;
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[0].pBufferInfo = &count_buffer_binding.info;

        desc_writes[1] = vku::InitStructHelper();
        desc_writes[1].dstSet = desc_set;
        desc_writes[1].dstBinding = dispatch_indirect_buffer_binding.binding;
        desc_writes[1].dstArrayElement = 0;
        desc_writes[1].descriptorCount = 1;
        desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[1].pBufferInfo = &dispatch_indirect_buffer_binding.info;

        return desc_writes;
    }
};

// Use "api_" prefix to make it clear which buffer/offset/etc we are talking about
// "api" helps to distinguish it is input from the user at the API level
void DrawIndexedIndirectIndexBuffer(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc, VkBuffer api_buffer,
                                    VkDeviceSize api_offset, uint32_t api_stride, uint32_t api_draw_count,
                                    VkBuffer api_count_buffer, VkDeviceSize api_count_buffer_offset, const char *vuid) {
    if (!gpuav.gpuav_settings.validate_index_buffers) {
        return;
    }

    if (gpuav.modified_features.robustBufferAccess2) {
        return;
    }

    if (gpuav.enabled_features.pipelineRobustness) {
        const LvlBindPoint lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
        const LastBound &last_bound = cb_state.base.lastBound[lv_bind_point];
        const vvl::Pipeline *pipeline_state = last_bound.pipeline_state;
        if (pipeline_state) {
            const auto robustness_ci =
                vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfo>(pipeline_state->GraphicsCreateInfo().pNext);
            if (robustness_ci && robustness_ci->vertexInputs) {
                return;
            }
        }
    }

    if (cb_state.max_actions_cmd_validation_reached_) {
        return;
    }

    if (!cb_state.base.IsPrimary()) {
        // TODO Unhandled for now. Potential issues with accessing the right vertex buffers
        // in secondary command buffers
        return;
    }

    if (!cb_state.base.index_buffer_binding.buffer) {
        return;
    }

    ValidationCommandFunc validation_cmd = [index_buffer_binding = cb_state.base.index_buffer_binding, api_buffer, api_offset,
                                            api_stride, api_draw_count, api_count_buffer, api_count_buffer_offset,
                                            draw_i = cb_state.draw_index,
                                            error_logger_i = uint32_t(cb_state.per_command_error_loggers.size()),
                                            loc](Validator &gpuav, CommandBufferSubState &cb_state) {
        SharedDrawValidationResources &shared_draw_validation_resources =
            gpuav.shared_resources_manager.GetOrCreate<SharedDrawValidationResources>(gpuav);
        if (!shared_draw_validation_resources.valid) {
            return;
        }
        valpipe::ComputePipeline<SetupDrawCountDispatchIndirectShader> &setup_validation_dispatch_pipeline =
            gpuav.shared_resources_manager.GetOrCreate<valpipe::ComputePipeline<SetupDrawCountDispatchIndirectShader>>(gpuav, loc);
        if (!setup_validation_dispatch_pipeline.valid) {
            return;
        }
        valpipe::ComputePipeline<DrawIndexedIndirectIndexBufferShader> &validation_pipeline =
            gpuav.shared_resources_manager.GetOrCreate<valpipe::ComputePipeline<DrawIndexedIndirectIndexBufferShader>>(
                gpuav, loc, cb_state.GetErrorLoggingDescSetLayout());
        if (!validation_pipeline.valid) {
            return;
        }

        const uint32_t index_bits_size = GetIndexBitsSize(index_buffer_binding.index_type);
        const uint32_t max_indices_in_buffer = static_cast<uint32_t>(index_buffer_binding.size / (index_bits_size / 8u));

        vko::BufferRange validation_dispatch_params_buffer_range =
            cb_state.gpu_resources_manager.GetDeviceLocalIndirectBufferRange(3 * sizeof(uint32_t));

        if (validation_dispatch_params_buffer_range.buffer == VK_NULL_HANDLE) {
            return;
        }

        glsl::DrawIndexedIndirectIndexBufferPushData push_constants{};
        if (api_count_buffer != VK_NULL_HANDLE) {
            push_constants.flags |= glsl::kIndexedIndirectDrawFlags_DrawCountFromBuffer;
            push_constants.api_count_buffer_offset_dwords = uint32_t(api_count_buffer_offset / sizeof(uint32_t));
        }
        push_constants.api_stride_dwords = api_stride / sizeof(uint32_t);
        push_constants.bound_index_buffer_indices_count = max_indices_in_buffer;
        push_constants.api_draw_count = api_draw_count;
        push_constants.api_offset_dwords = uint32_t(api_offset / sizeof(uint32_t));

        // Draw count being stored in a GPU buffer,
        // setup a compute pipeline to determine the size of the validation indirect dispatch
        {
            SetupDrawCountDispatchIndirectShader setup_validation_shader_resources;
            setup_validation_shader_resources.push_constants = push_constants;
            if (api_count_buffer != VK_NULL_HANDLE) {
                setup_validation_shader_resources.count_buffer_binding.info = {api_count_buffer, 0, sizeof(uint32_t)};
            } else {
                setup_validation_shader_resources.count_buffer_binding.info = {
                    shared_draw_validation_resources.dummy_buffer.VkHandle(), 0, VK_WHOLE_SIZE};
            }

            setup_validation_shader_resources.dispatch_indirect_buffer_binding.info = {
                validation_dispatch_params_buffer_range.buffer, validation_dispatch_params_buffer_range.offset,
                validation_dispatch_params_buffer_range.size};

            if (!setup_validation_dispatch_pipeline.BindShaderResources(gpuav, cb_state, setup_validation_shader_resources)) {
                return;
            }

            DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE,
                                    setup_validation_dispatch_pipeline.pipeline);

            // Sync indirect buffer writes - the same command buffer could be executed concurrently
            // for all we know
            {
                VkBufferMemoryBarrier barrier_write_after_read = vku::InitStructHelper();
                barrier_write_after_read.srcAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
                barrier_write_after_read.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier_write_after_read.buffer = validation_dispatch_params_buffer_range.buffer;
                barrier_write_after_read.offset = validation_dispatch_params_buffer_range.offset;
                barrier_write_after_read.size = validation_dispatch_params_buffer_range.size;

                DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &barrier_write_after_read, 0,
                                           nullptr);
            }

            DispatchCmdDispatch(cb_state.VkHandle(), 1, 1, 1);

            // Sync indirect buffer reads
            {
                VkBufferMemoryBarrier barrier_read_after_write = vku::InitStructHelper();
                barrier_read_after_write.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier_read_after_write.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
                barrier_read_after_write.buffer = validation_dispatch_params_buffer_range.buffer;
                barrier_read_after_write.offset = validation_dispatch_params_buffer_range.offset;
                barrier_read_after_write.size = validation_dispatch_params_buffer_range.size;

                DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                           VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, 0, 0, nullptr, 1, &barrier_read_after_write, 0,
                                           nullptr);
            }
        }

        // Setup validation pipeline
        {
            DrawIndexedIndirectIndexBufferShader validation_shader_resources;
            validation_shader_resources.push_constants = push_constants;
            if (api_count_buffer != VK_NULL_HANDLE) {
                validation_shader_resources.count_buffer_binding.info = {api_count_buffer, 0, sizeof(uint32_t)};
            } else {
                validation_shader_resources.count_buffer_binding.info = {shared_draw_validation_resources.dummy_buffer.VkHandle(),
                                                                         0, VK_WHOLE_SIZE};
            }
            validation_shader_resources.draw_buffer_binding.info = {api_buffer, 0, VK_WHOLE_SIZE};

            if (!BindShaderResources(validation_pipeline, gpuav, cb_state, draw_i, error_logger_i, validation_shader_resources)) {
                return;
            }

            DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, validation_pipeline.pipeline);

            // One draw will check all VkDrawIndexedIndirectCommand
            DispatchCmdDispatchIndirect(cb_state.VkHandle(), validation_dispatch_params_buffer_range.buffer,
                                        validation_dispatch_params_buffer_range.offset);
            // synchronize draw buffer validation (read) against subsequent writes
            std::array<VkBufferMemoryBarrier, 2> buffer_memory_barriers = {};
            uint32_t buffer_memory_barriers_count = 1;
            buffer_memory_barriers[0] = vku::InitStructHelper();
            buffer_memory_barriers[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            buffer_memory_barriers[0].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            buffer_memory_barriers[0].buffer = api_buffer;
            buffer_memory_barriers[0].offset = api_offset;
            buffer_memory_barriers[0].size = VK_WHOLE_SIZE;

            if (api_count_buffer) {
                buffer_memory_barriers[1] = vku::InitStructHelper();
                buffer_memory_barriers[1].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                buffer_memory_barriers[1].dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                buffer_memory_barriers[1].buffer = api_count_buffer;
                buffer_memory_barriers[1].offset = api_count_buffer_offset;
                buffer_memory_barriers[1].size = sizeof(uint32_t);
                ++buffer_memory_barriers_count;
            }

            DispatchCmdPipelineBarrier(cb_state.VkHandle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, buffer_memory_barriers_count,
                                       buffer_memory_barriers.data(), 0, nullptr);
        }
    };
    cb_state.per_render_pass_validation_commands.emplace_back(std::move(validation_cmd));

    const uint32_t label_command_i =
        !cb_state.base.GetLabelCommands().empty() ? uint32_t(cb_state.base.GetLabelCommands().size() - 1) : vvl::kU32Max;
    ErrorLoggerFunc error_logger = [&gpuav, &cb_state, loc, vuid, api_buffer, api_offset, api_stride,
                                    index_buffer_binding = cb_state.base.index_buffer_binding,
                                    label_command_i](const uint32_t *error_record, const LogObjectList &objlist,
                                                     const std::vector<std::string> &initial_label_stack) {
        bool skip = false;
        using namespace glsl;

        const uint32_t error_sub_code = (error_record[kHeaderShaderIdErrorOffset] & kErrorSubCodeMask) >> kErrorSubCodeShift;
        switch (error_sub_code) {
            case kErrorSubCode_OobIndexBuffer: {
                const uint32_t draw_i = error_record[kPreActionParamOffset_0];
                const uint32_t first_index = error_record[kPreActionParamOffset_1];
                const uint32_t index_count = error_record[kPreActionParamOffset_2];
                const uint32_t highest_accessed_index = first_index + index_count;
                const uint32_t index_bits_size = GetIndexBitsSize(index_buffer_binding.index_type);
                const uint32_t max_indices_in_buffer = static_cast<uint32_t>(index_buffer_binding.size / (index_bits_size / 8u));

                std::string debug_region_name = cb_state.GetDebugLabelRegion(label_command_i, initial_label_stack);
                Location loc_with_debug_region(loc, debug_region_name);
                skip |= gpuav.LogError(
                    vuid, objlist, loc_with_debug_region,
                    "Index %" PRIu32 " is not within the bound index buffer. Computed from VkDrawIndexedIndirectCommand[%" PRIu32
                    "] (.firstIndex = %" PRIu32 ", .indexCount = %" PRIu32
                    ")\n"

                    "VkDrawIndexedIndirectCommand buffer:\n"
                    "- Buffer: %s\n"
                    "- Buffer offset: %" PRIu64
                    "\n"

                    "Index buffer binding info:\n"
                    "- Buffer: %s\n"
                    "- Index type: %s\n"
                    "- Binding offset: %" PRIu64
                    "\n"
                    "- Binding size: %" PRIu64 " bytes (or %" PRIu32
                    " %s)\n"

                    "Supplied buffer parameters in indirect command: offset = %" PRIu64 ", stride = %" PRIu32 " bytes.",
                    // OOB index info
                    highest_accessed_index, draw_i, first_index, index_count,

                    // Draw parameters buffer
                    gpuav.FormatHandle(api_buffer).c_str(), api_offset,

                    // Index buffer binding info
                    gpuav.FormatHandle(index_buffer_binding.buffer).c_str(), string_VkIndexType(index_buffer_binding.index_type),
                    index_buffer_binding.offset, index_buffer_binding.size, max_indices_in_buffer,
                    string_VkIndexType(index_buffer_binding.index_type),

                    // VkDrawIndexedIndirectCommand info
                    api_offset, api_stride);
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
