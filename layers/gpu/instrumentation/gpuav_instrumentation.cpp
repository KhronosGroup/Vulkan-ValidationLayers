/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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

#include "gpu/instrumentation/gpuav_instrumentation.h"

#include "chassis/chassis_modification_state.h"
#include "gpu/core/gpuav.h"
#include "gpu/resources/gpuav_subclasses.h"
#include "gpu/spirv/module.h"
#include "state_tracker/shader_stage_state.h"
#include "spirv-tools/optimizer.hpp"

#include <fstream>

namespace gpuav {

static bool GpuValidateShader(const std::vector<uint32_t> &input, bool SetRelaxBlockLayout, bool SetScalerBlockLayout,
                              spv_target_env target_env, std::string &error) {
    // Use SPIRV-Tools validator to try and catch any issues with the module
    spv_context ctx = spvContextCreate(target_env);
    spv_const_binary_t binary{input.data(), input.size()};
    spv_diagnostic diag = nullptr;
    spv_validator_options options = spvValidatorOptionsCreate();
    spvValidatorOptionsSetRelaxBlockLayout(options, SetRelaxBlockLayout);
    spvValidatorOptionsSetScalarBlockLayout(options, SetScalerBlockLayout);
    spv_result_t result = spvValidateWithOptions(ctx, options, &binary, &diag);
    if (result != SPV_SUCCESS && diag) error = diag->error;
    return (result == SPV_SUCCESS);
}

// Call the SPIR-V Optimizer to run the instrumentation pass on the shader.
bool Validator::InstrumentShader(const vvl::span<const uint32_t> &input, uint32_t unique_shader_id, const Location &loc,
                                 std::vector<uint32_t> &out_instrumented_spirv) {
    if (input[0] != spv::MagicNumber) return false;

    const spvtools::MessageConsumer gpu_console_message_consumer =
        [this, loc](spv_message_level_t level, const char *, const spv_position_t &position, const char *message) -> void {
        switch (level) {
            case SPV_MSG_FATAL:
            case SPV_MSG_INTERNAL_ERROR:
            case SPV_MSG_ERROR:
                this->LogError("UNASSIGNED-GPU-Assisted", this->device, loc, "Error during shader instrumentation: line %zu: %s",
                               position.index, message);
                break;
            default:
                break;
        }
    };
    std::vector<std::vector<uint32_t>> binaries(2);

    // Load original shader SPIR-V
    binaries[0].reserve(input.size());
    binaries[0].insert(binaries[0].end(), &input.front(), &input.back() + 1);

    if (gpuav_settings.debug_dump_instrumented_shaders) {
        std::string file_name = "dump_" + std::to_string(unique_shader_id) + "_before.spv";
        std::ofstream debug_file(file_name, std::ios::out | std::ios::binary);
        debug_file.write(reinterpret_cast<char *>(binaries[0].data()),
                         static_cast<std::streamsize>(binaries[0].size() * sizeof(uint32_t)));
    }

    spv_target_env target_env = PickSpirvEnv(api_version, IsExtEnabled(device_extensions.vk_khr_spirv_1_4));

    // Use the unique_shader_id as a shader ID so we can look up its handle later in the shader_map.
    spirv::Module module(binaries[0], unique_shader_id, desc_set_bind_index_);

    // If descriptor indexing is enabled, enable length checks and updated descriptor checks
    if (gpuav_settings.validate_descriptors) {
        module.RunPassBindlessDescriptor();
    }

    if (gpuav_settings.validate_bda) {
        module.RunPassBufferDeviceAddress();
    }

    if (gpuav_settings.validate_ray_query) {
        module.RunPassRayQuery();
    }

    for (const auto info : module.link_info_) {
        module.LinkFunction(info);
    }

    module.ToBinary(out_instrumented_spirv);

    if (gpuav_settings.debug_dump_instrumented_shaders) {
        std::string file_name = "dump_" + std::to_string(unique_shader_id) + "_after.spv";
        std::ofstream debug_file(file_name, std::ios::out | std::ios::binary);
        debug_file.write(reinterpret_cast<char *>(out_instrumented_spirv.data()),
                         static_cast<std::streamsize>(out_instrumented_spirv.size() * sizeof(uint32_t)));
    }

    // (Maybe) validate the instrumented and linked shader
    if (gpuav_settings.debug_validate_instrumented_shaders) {
        std::string instrumented_error;
        if (!GpuValidateShader(out_instrumented_spirv, device_extensions.vk_khr_relaxed_block_layout,
                               device_extensions.vk_ext_scalar_block_layout, target_env, instrumented_error)) {
            std::ostringstream strm;
            strm << "Instrumented shader (id " << unique_shader_id << ") is invalid, spirv-val error:\n"
                 << instrumented_error << " Proceeding with non instrumented shader.";
            InternalError(device, loc, strm.str().c_str());
            assert(false);
            return false;
        }
    }
    // Run Dead Code elimination
    {
        using namespace spvtools;
        OptimizerOptions opt_options;
        opt_options.set_run_validator(false);
        Optimizer dce_pass(target_env);
        dce_pass.SetMessageConsumer(gpu_console_message_consumer);
        // Call CreateAggressiveDCEPass with preserve_interface == true
        dce_pass.RegisterPass(CreateAggressiveDCEPass(true));
        if (!dce_pass.Run(out_instrumented_spirv.data(), out_instrumented_spirv.size(), &out_instrumented_spirv, opt_options)) {
            InternalError(device, loc,
                          "Failure to run spirv-opt DCE on instrumented shader.  Proceeding with non-instrumented shader.");
            assert(false);
            return false;
        }

        if (gpuav_settings.debug_dump_instrumented_shaders) {
            std::string file_name = "dump_" + std::to_string(unique_shader_id) + "_opt.spv";
            std::ofstream debug_file(file_name, std::ios::out | std::ios::binary);
            debug_file.write(reinterpret_cast<char *>(out_instrumented_spirv.data()),
                             static_cast<std::streamsize>(out_instrumented_spirv.size() * sizeof(uint32_t)));
        }
    }

    return true;
}

CommandResources Validator::SetupShaderInstrumentationResources(const LockedSharedPtr<CommandBuffer, WriteLockGuard> &cmd_buffer,
                                                                VkPipelineBindPoint bind_point, const Location &loc) {
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        assert(false);
        return CommandResources();
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    auto const &last_bound = cmd_buffer->lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;

    if (!pipeline_state && !last_bound.HasShaderObjects()) {
        InternalError(cmd_buffer->VkHandle(), loc, "Neither pipeline state nor shader object states were found, aborting GPU-AV");
        return CommandResources();
    }

    VkDescriptorSet instrumentation_desc_set = VK_NULL_HANDLE;
    VkDescriptorPool instrumentation_desc_pool = VK_NULL_HANDLE;
    VkResult result = desc_set_manager_->GetDescriptorSet(
        &instrumentation_desc_pool, cmd_buffer->GetInstrumentationDescriptorSetLayout(), &instrumentation_desc_set);
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        InternalError(cmd_buffer->VkHandle(), loc,
                      "Unable to allocate instrumentation descriptor sets. Device could become unstable.");
        return CommandResources();
    }

    // Update instrumentation descriptor set
    {
        // Pathetic way of trying to make sure we take care of updating all
        // bindings of the instrumentation descriptor set
        assert(instrumentation_bindings_.size() == 6);
        std::vector<VkWriteDescriptorSet> desc_writes = {};

        // Error output buffer
        VkDescriptorBufferInfo error_output_desc_buffer_info = {};
        {
            error_output_desc_buffer_info.range = VK_WHOLE_SIZE;
            error_output_desc_buffer_info.buffer = cmd_buffer->GetErrorOutputBuffer();
            error_output_desc_buffer_info.offset = 0;

            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstErrorBuffer;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            wds.pBufferInfo = &error_output_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        // Buffer holding action command index in command buffer
        VkDescriptorBufferInfo indices_desc_buffer_info = {};
        {
            indices_desc_buffer_info.range = sizeof(uint32_t);
            indices_desc_buffer_info.buffer = indices_buffer_.buffer;
            indices_desc_buffer_info.offset = 0;

            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstActionIndex;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            wds.pBufferInfo = &indices_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        // Buffer holding a resource index from the per command buffer command resources list
        {
            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstCmdResourceIndex;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            wds.pBufferInfo = &indices_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        // Errors count buffer
        VkDescriptorBufferInfo cmd_errors_counts_desc_buffer_info = {};
        {
            cmd_errors_counts_desc_buffer_info.range = VK_WHOLE_SIZE;
            cmd_errors_counts_desc_buffer_info.buffer = cmd_buffer->GetCmdErrorsCountsBuffer();
            cmd_errors_counts_desc_buffer_info.offset = 0;

            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstCmdErrorsCount;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            wds.pBufferInfo = &cmd_errors_counts_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        // Current bindless buffer
        VkDescriptorBufferInfo di_input_desc_buffer_info = {};
        if (cmd_buffer->current_bindless_buffer != VK_NULL_HANDLE) {
            di_input_desc_buffer_info.range = VK_WHOLE_SIZE;
            di_input_desc_buffer_info.buffer = cmd_buffer->current_bindless_buffer;
            di_input_desc_buffer_info.offset = 0;

            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstBindlessDescriptor;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            wds.pBufferInfo = &di_input_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        // BDA snapshot buffer
        VkDescriptorBufferInfo bda_input_desc_buffer_info = {};
        if (bda_validation_possible_) {
            bda_input_desc_buffer_info.range = VK_WHOLE_SIZE;
            bda_input_desc_buffer_info.buffer = cmd_buffer->GetBdaRangesSnapshot().buffer;
            bda_input_desc_buffer_info.offset = 0;

            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstBufferDeviceAddress;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            wds.pBufferInfo = &bda_input_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        DispatchUpdateDescriptorSets(device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, nullptr);
    }

    const auto pipeline_layout =
        pipeline_state ? pipeline_state->PipelineLayoutState() : Get<vvl::PipelineLayout>(last_bound.pipeline_layout);
    // If GPL is used, it's possible the pipeline layout used at pipeline creation time is null. If CmdBindDescriptorSets has
    // not been called yet (i.e., state.pipeline_null), then fall back to the layout associated with pre-raster state.
    // PipelineLayoutState should be used for the purposes of determining the number of sets in the layout, but this layout
    // may be a "pseudo layout" used to represent the union of pre-raster and fragment shader layouts, and therefore have a
    // null handle.
    VkPipelineLayout pipeline_layout_handle = VK_NULL_HANDLE;
    if (last_bound.pipeline_layout) {
        pipeline_layout_handle = last_bound.pipeline_layout;
    } else if (pipeline_state && !pipeline_state->PreRasterPipelineLayoutState()->Destroyed()) {
        pipeline_layout_handle = pipeline_state->PreRasterPipelineLayoutState()->VkHandle();
    }

    CommandResources cmd_resources;
    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS)
        cmd_resources.operation_index = cmd_buffer->draw_index++;
    else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE)
        cmd_resources.operation_index = cmd_buffer->compute_index++;
    else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)
        cmd_resources.operation_index = cmd_buffer->trace_rays_index++;

    // Using cmd_buffer->per_command_resources.size() is kind of a hack? Worth considering passing the resource index as a parameter
    const std::array<uint32_t, 2> dynamic_offsets = {
        {cmd_resources.operation_index * static_cast<uint32_t>(sizeof(uint32_t)),
         static_cast<uint32_t>(cmd_buffer->per_command_resources.size()) * static_cast<uint32_t>(sizeof(uint32_t))}};
    if ((pipeline_layout && pipeline_layout->set_layouts.size() <= desc_set_bind_index_) &&
        pipeline_layout_handle != VK_NULL_HANDLE) {
        DispatchCmdBindDescriptorSets(cmd_buffer->VkHandle(), bind_point, pipeline_layout_handle, desc_set_bind_index_, 1,
                                      &instrumentation_desc_set, static_cast<uint32_t>(dynamic_offsets.size()),
                                      dynamic_offsets.data());
    } else {
        // If no pipeline layout was bound when using shader objects that don't use any descriptor set, bind the debug pipeline
        // layout
        DispatchCmdBindDescriptorSets(cmd_buffer->VkHandle(), bind_point, GetDebugPipelineLayout(), desc_set_bind_index_, 1,
                                      &instrumentation_desc_set, static_cast<uint32_t>(dynamic_offsets.size()),
                                      dynamic_offsets.data());
    }

    if (pipeline_state && pipeline_layout_handle == VK_NULL_HANDLE) {
        InternalError(cmd_buffer->Handle(), loc, "Unable to find pipeline layout to bind debug descriptor set. Aborting GPU-AV");
        return CommandResources();
    }

    // It is possible to have no descriptor sets bound, for example if using push constants.
    const uint32_t di_buf_index =
        !cmd_buffer->di_input_buffer_list.empty() ? uint32_t(cmd_buffer->di_input_buffer_list.size()) - 1 : vvl::kU32Max;

    const bool uses_robustness = (enabled_features.robustBufferAccess || enabled_features.robustBufferAccess2 ||
                                  (pipeline_state && pipeline_state->uses_pipeline_robustness));

    cmd_resources.instrumentation_desc_set = instrumentation_desc_set;
    cmd_resources.instrumentation_desc_pool = instrumentation_desc_pool;
    cmd_resources.pipeline_bind_point = bind_point;
    cmd_resources.uses_robustness = uses_robustness;
    cmd_resources.uses_shader_object = pipeline_state == nullptr;
    cmd_resources.command = loc.function;
    cmd_resources.desc_binding_index = di_buf_index;
    cmd_resources.desc_binding_list = &cmd_buffer->di_input_buffer_list;
    return cmd_resources;
}

CommandResources Validator::SetupShaderInstrumentationResources(VkCommandBuffer cmd_buffer, VkPipelineBindPoint bind_point,
                                                                const Location &loc) {
    auto cb_state = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_state) {
        InternalError(cmd_buffer, loc, "Unrecognized command buffer");
        return CommandResources();
    }
    return SetupShaderInstrumentationResources(cb_state, bind_point, loc);
}

}  // namespace gpuav
