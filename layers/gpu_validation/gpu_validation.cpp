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

#include <cmath>
#include <fstream>
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <unistd.h>
#endif
#include "utils/cast_utils.h"
#include "utils/shader_utils.h"
#include "utils/hash_util.h"
#include "gpu_validation/gpu_constants.h"
#include "gpu_validation/gpu_subclasses.h"
#include "gpu_validation/gpu_validation.h"
#include "spirv-tools/instrument.hpp"
#include "spirv-tools/linker.hpp"
#include "generated/layer_chassis_dispatch.h"
#include "gpu_vuids.h"
#include "containers/custom_containers.h"
#include "spirv/module.h"
#include "chassis/chassis_modification_state.h"
#include "state_tracker/render_pass_state.h"
// Generated shaders
#include "gpu_shaders/gpu_error_header.h"
#include "generated/gpu_pre_draw_vert.h"
#include "generated/gpu_pre_dispatch_comp.h"
#include "generated/gpu_pre_trace_rays_rgen.h"
#include "generated/gpu_inst_shader_hash.h"
#include "generated/gpu_pre_copy_buffer_to_image_comp.h"

namespace gpuav {

VkDeviceAddress Validator::GetBufferDeviceAddress(VkBuffer buffer, const Location &loc) const {
    if (!enabled_features.bufferDeviceAddress) {
        assert(false);
        ReportSetupProblem(buffer, loc, "bufferDeviceAddress feature not enabled, calling GetBufferDeviceAddress is invalid.");
        aborted = true;
        return 0;
    }

    VkBufferDeviceAddressInfo address_info = vku::InitStructHelper();
    address_info.buffer = buffer;
    if (api_version >= VK_API_VERSION_1_2) {
        return DispatchGetBufferDeviceAddress(device, &address_info);
    }
    if (IsExtEnabled(device_extensions.vk_ext_buffer_device_address)) {
        return DispatchGetBufferDeviceAddressEXT(device, &address_info);
    }
    if (IsExtEnabled(device_extensions.vk_khr_buffer_device_address)) {
        return DispatchGetBufferDeviceAddressKHR(device, &address_info);
    }
    return 0;
}

bool Validator::CheckForDescriptorIndexing(DeviceFeatures enabled_features) const {
    // If no features are enabled, descriptor indexing is not being used actually
    bool result =
        enabled_features.descriptorIndexing || enabled_features.shaderInputAttachmentArrayDynamicIndexing ||
        enabled_features.shaderUniformTexelBufferArrayDynamicIndexing ||
        enabled_features.shaderStorageTexelBufferArrayDynamicIndexing ||
        enabled_features.shaderUniformBufferArrayNonUniformIndexing || enabled_features.shaderSampledImageArrayNonUniformIndexing ||
        enabled_features.shaderStorageBufferArrayNonUniformIndexing || enabled_features.shaderStorageImageArrayNonUniformIndexing ||
        enabled_features.shaderInputAttachmentArrayNonUniformIndexing ||
        enabled_features.shaderUniformTexelBufferArrayNonUniformIndexing ||
        enabled_features.shaderStorageTexelBufferArrayNonUniformIndexing ||
        enabled_features.descriptorBindingUniformBufferUpdateAfterBind ||
        enabled_features.descriptorBindingSampledImageUpdateAfterBind ||
        enabled_features.descriptorBindingStorageImageUpdateAfterBind ||
        enabled_features.descriptorBindingStorageBufferUpdateAfterBind ||
        enabled_features.descriptorBindingUniformTexelBufferUpdateAfterBind ||
        enabled_features.descriptorBindingStorageTexelBufferUpdateAfterBind ||
        enabled_features.descriptorBindingUpdateUnusedWhilePending || enabled_features.descriptorBindingPartiallyBound ||
        enabled_features.descriptorBindingVariableDescriptorCount || enabled_features.runtimeDescriptorArray;
    return result;
}

static bool GpuValidateShader(const vvl::span<const uint32_t> &input, bool SetRelaxBlockLayout, bool SetScalerBlockLayout,
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
bool Validator::InstrumentShader(const vvl::span<const uint32_t> &input, std::vector<uint32_t> &instrumented_spirv,
                                 const uint32_t unique_shader_id, const Location &loc) {
    if (aborted) return false;
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

    using namespace spvtools;
    spv_target_env target_env = PickSpirvEnv(api_version, IsExtEnabled(device_extensions.vk_khr_spirv_1_4));

    // Use the unique_shader_id as a shader ID so we can look up its handle later in the shader_map.
    spirv::Module module(binaries[0], unique_shader_id, desc_set_bind_index);

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

    module.ToBinary(instrumented_spirv);

    if (gpuav_settings.debug_dump_instrumented_shaders) {
        std::string file_name = "dump_" + std::to_string(unique_shader_id) + "_after.spv";
        std::ofstream debug_file(file_name, std::ios::out | std::ios::binary);
        debug_file.write(reinterpret_cast<char *>(instrumented_spirv.data()),
                         static_cast<std::streamsize>(instrumented_spirv.size() * sizeof(uint32_t)));
    }

    // (Maybe) validate the instrumented and linked shader
    if (gpuav_settings.debug_validate_instrumented_shaders) {
        std::string instrumented_error;
        if (!GpuValidateShader(instrumented_spirv, device_extensions.vk_khr_relaxed_block_layout,
                               device_extensions.vk_ext_scalar_block_layout, target_env, instrumented_error)) {
            std::ostringstream strm;
            strm << "Instrumented shader (id " << unique_shader_id << ") is invalid, spirv-val error:\n"
                 << instrumented_error << " Proceeding with non instrumented shader.";
            ReportSetupProblem(device, loc, strm.str().c_str());
            assert(false);
            return false;
        }
    }
    // Run Dead Code elimination
    {
        OptimizerOptions opt_options;
        opt_options.set_run_validator(false);
        Optimizer dce_pass(target_env);
        dce_pass.SetMessageConsumer(gpu_console_message_consumer);
        // Call CreateAggressiveDCEPass with preserve_interface == true
        dce_pass.RegisterPass(CreateAggressiveDCEPass(true));
        if (!dce_pass.Run(instrumented_spirv.data(), instrumented_spirv.size(), &instrumented_spirv, opt_options)) {
            ReportSetupProblem(device, loc,
                               "Failure to run spirv-opt DCE on instrumented shader.  Proceeding with non-instrumented shader.");
            assert(false);
            return false;
        }

        if (gpuav_settings.debug_dump_instrumented_shaders) {
            std::string file_name = "dump_" + std::to_string(unique_shader_id) + "_opt.spv";
            std::ofstream debug_file(file_name, std::ios::out | std::ios::binary);
            debug_file.write(reinterpret_cast<char *>(instrumented_spirv.data()),
                             static_cast<std::streamsize>(instrumented_spirv.size() * sizeof(uint32_t)));
        }
    }

    return true;
}

bool Validator::CheckForCachedInstrumentedShader(uint32_t shader_hash, chassis::CreateShaderModule &chassis_state) {
    auto it = instrumented_shaders.find(shader_hash);
    if (it != instrumented_shaders.end()) {
        chassis_state.instrumented_create_info.codeSize = it->second.first * sizeof(uint32_t);
        chassis_state.instrumented_create_info.pCode = it->second.second.data();
        chassis_state.instrumented_spirv = it->second.second;
        chassis_state.unique_shader_id = shader_hash;
        return true;
    }
    return false;
}

bool Validator::CheckForCachedInstrumentedShader(uint32_t index, uint32_t shader_hash, chassis::ShaderObject &chassis_state) {
    auto it = instrumented_shaders.find(shader_hash);
    if (it != instrumented_shaders.end()) {
        chassis_state.instrumented_create_info[index].codeSize = it->second.first * sizeof(uint32_t);
        chassis_state.instrumented_create_info[index].pCode = it->second.second.data();
        return true;
    }
    return false;
}

// For the given command buffer, map its debug data buffers and update the status of any update after bind descriptors
void Validator::UpdateInstrumentationBuffer(CommandBuffer *cb_node) {
    for (auto &cmd_info : cb_node->di_input_buffer_list) {
        glsl::BindlessStateBuffer *bindless_state{nullptr};
        [[maybe_unused]] VkResult result;
        result = vmaMapMemory(vmaAllocator, cmd_info.bindless_state_buffer_allocation, reinterpret_cast<void **>(&bindless_state));
        assert(result == VK_SUCCESS);
        assert(bindless_state->global_state == desc_heap->GetDeviceAddress());
        for (size_t i = 0; i < cmd_info.descriptor_set_buffers.size(); i++) {
            auto &set_buffer = cmd_info.descriptor_set_buffers[i];
            bindless_state->desc_sets[i].layout_data = set_buffer.state->GetLayoutState();
            if (!set_buffer.gpu_state) {
                set_buffer.gpu_state = set_buffer.state->GetCurrentState();
                bindless_state->desc_sets[i].in_data = set_buffer.gpu_state->device_addr;
            }
            if (!set_buffer.output_state) {
                set_buffer.output_state = set_buffer.state->GetOutputState();
                bindless_state->desc_sets[i].out_data = set_buffer.output_state->device_addr;
            }
        }
        vmaUnmapMemory(vmaAllocator, cmd_info.bindless_state_buffer_allocation);
    }
}

void Validator::UpdateBoundPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                    const Location &loc) {
    if (aborted) return;
    if (!gpuav_settings.validate_descriptors) return;

    auto cb_node = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_node) {
        ReportSetupProblem(commandBuffer, loc, "Unrecognized command buffer");
        aborted = true;
        return;
    }
    const auto lv_bind_point = ConvertToLvlBindPoint(pipelineBindPoint);
    auto const &last_bound = cb_node->lastBound[lv_bind_point];
    // Should have just been updated
    if (!last_bound.pipeline_state) {
        ReportSetupProblem(pipeline, loc, "Unrecognized pipeline");
        aborted = true;
        return;
    }

    // Catch if pipeline is bound before any descriptor sets
    if (cb_node->di_input_buffer_list.empty()) {
        return;
    }

    // Update the last vkCmdBindDescriptorSet with the new pipeline
    auto &descriptor_set_buffers = cb_node->di_input_buffer_list.back().descriptor_set_buffers;

    // If the user calls vkCmdBindDescriptorSet::firstSet to a non-zero value, these indexes don't line up
    size_t update_index = 0;
    for (uint32_t i = 0; i < last_bound.per_set.size(); i++) {
        if (last_bound.per_set[i].bound_descriptor_set) {
            auto slot = last_bound.pipeline_state->active_slots.find(i);
            if (slot != last_bound.pipeline_state->active_slots.end()) {
                if (update_index >= descriptor_set_buffers.size()) {
                    // TODO - Hit crash running with Dota2, this shouldn't happen, need to look into
                    continue;
                }
                descriptor_set_buffers[update_index++].binding_req = slot->second;
            }
        }
    }
}

void Validator::UpdateBoundDescriptors(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, const Location &loc) {
    if (aborted) return;
    auto cb_node = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_node) {
        ReportSetupProblem(commandBuffer, loc, "Unrecognized command buffer");
        aborted = true;
        return;
    }
    const auto lv_bind_point = ConvertToLvlBindPoint(pipelineBindPoint);
    auto const &last_bound = cb_node->lastBound[lv_bind_point];

    uint32_t number_of_sets = static_cast<uint32_t>(last_bound.per_set.size());
    // Figure out how much memory we need for the input block based on how many sets and bindings there are
    // and how big each of the bindings is
    if (number_of_sets > 0 && gpuav_settings.validate_descriptors && force_buffer_device_address) {
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        assert(number_of_sets <= glsl::kDebugInputBindlessMaxDescSets);
        buffer_info.size = sizeof(glsl::BindlessStateBuffer);
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_info.pool = VK_NULL_HANDLE;
        DescBindingInfo di_buffers = {};

        // Allocate buffer for device addresses of the input buffer for each descriptor set.  This is the buffer written to each
        // draw's descriptor set.
        VkResult result = vmaCreateBuffer(vmaAllocator, &buffer_info, &alloc_info, &di_buffers.bindless_state_buffer,
                                          &di_buffers.bindless_state_buffer_allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(commandBuffer, loc, "Unable to allocate device memory. Device could become unstable.", true);
            aborted = true;
            return;
        }
        glsl::BindlessStateBuffer *bindless_state{nullptr};
        result =
            vmaMapMemory(vmaAllocator, di_buffers.bindless_state_buffer_allocation, reinterpret_cast<void **>(&bindless_state));
        if (result != VK_SUCCESS) {
            ReportSetupProblem(commandBuffer, loc, "Unable to map device memory. Device could become unstable.", true);
            aborted = true;
            return;
        }
        memset(bindless_state, 0, static_cast<size_t>(buffer_info.size));
        cb_node->current_bindless_buffer = di_buffers.bindless_state_buffer;

        bindless_state->global_state = desc_heap->GetDeviceAddress();
        for (uint32_t i = 0; i < last_bound.per_set.size(); i++) {
            const auto &s = last_bound.per_set[i];
            auto set = s.bound_descriptor_set;
            if (!set) {
                continue;
            }
            if (gpuav_settings.validate_descriptors) {
                DescSetState desc_set_state;
                desc_set_state.num = i;
                desc_set_state.state = std::static_pointer_cast<DescriptorSet>(set);
                bindless_state->desc_sets[i].layout_data = desc_set_state.state->GetLayoutState();
                // The pipeline might not have been bound yet, so will need to update binding_req later
                if (last_bound.pipeline_state) {
                    auto slot = last_bound.pipeline_state->active_slots.find(i);
                    if (slot != last_bound.pipeline_state->active_slots.end()) {
                        desc_set_state.binding_req = slot->second;
                    }
                }
                if (!desc_set_state.state->IsUpdateAfterBind()) {
                    desc_set_state.gpu_state = desc_set_state.state->GetCurrentState();
                    bindless_state->desc_sets[i].in_data = desc_set_state.gpu_state->device_addr;
                    desc_set_state.output_state = desc_set_state.state->GetOutputState();
                    bindless_state->desc_sets[i].out_data = desc_set_state.output_state->device_addr;
                }
                di_buffers.descriptor_set_buffers.emplace_back(std::move(desc_set_state));
            }
        }
        cb_node->di_input_buffer_list.emplace_back(di_buffers);
        vmaUnmapMemory(vmaAllocator, di_buffers.bindless_state_buffer_allocation);
    }
}

void Validator::BindDiagnosticCallsCommonDescSet(const LockedSharedPtr<CommandBuffer, WriteLockGuard> &cmd_buffer_state,
                                                 VkPipelineBindPoint bind_point, VkPipelineLayout pipeline_layout,
                                                 uint32_t cmd_index, uint32_t resource_index) {
    assert(cmd_index < cst::indices_count);
    assert(resource_index < cst::indices_count);
    std::array<uint32_t, 2> dynamic_offsets = {
        {cmd_index * static_cast<uint32_t>(sizeof(uint32_t)), resource_index * static_cast<uint32_t>(sizeof(uint32_t))}};
    DispatchCmdBindDescriptorSets(cmd_buffer_state->VkHandle(), bind_point, pipeline_layout, glsl::kDiagCommonDescriptorSet, 1,
                                  &cmd_buffer_state->GetValidationCmdCommonDescriptorSet(),
                                  static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
}

// Common resource allocations functions

CommandResources Validator::AllocateActionCommandResources(const LockedSharedPtr<CommandBuffer, WriteLockGuard> &cmd_buffer,
                                                           VkPipelineBindPoint bind_point, const Location &loc,
                                                           const CmdIndirectState *indirect_state /*= nullptr*/) {
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        assert(false);
        return CommandResources();
    }

    if (aborted) return CommandResources();

    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    auto const &last_bound = cmd_buffer->lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;

    if (!pipeline_state && !last_bound.HasShaderObjects()) {
        ReportSetupProblem(cmd_buffer->VkHandle(), loc,
                           "Neither pipeline state nor shader object states were found, aborting GPU-AV");
        aborted = true;
        return CommandResources();
    }

    VkDescriptorSet instrumentation_desc_set = VK_NULL_HANDLE;
    VkDescriptorPool instrumentation_desc_pool = VK_NULL_HANDLE;
    VkResult result = desc_set_manager->GetDescriptorSet(
        &instrumentation_desc_pool, cmd_buffer->GetInstrumentationDescriptorSetLayout(), &instrumentation_desc_set);
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(cmd_buffer->VkHandle(), loc,
                           "Unable to allocate instrumentation descriptor sets. Device could become unstable.");
        aborted = true;
        return CommandResources();
    }

    // Update instrumentation descriptor set
    {
        // Pathetic way of trying to make sure we take care of updating all
        // bindings of the instrumentation descriptor set
        assert(validation_bindings_.size() == 6);
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
            indices_desc_buffer_info.buffer = indices_buffer.buffer;
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

        // Buffer device addresses buffer
        VkDescriptorBufferInfo bda_input_desc_buffer_info = {};
        if (bda_validation_possible) {
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
    if ((pipeline_layout && pipeline_layout->set_layouts.size() <= desc_set_bind_index) &&
        pipeline_layout_handle != VK_NULL_HANDLE) {
        DispatchCmdBindDescriptorSets(cmd_buffer->VkHandle(), bind_point, pipeline_layout_handle, desc_set_bind_index, 1,
                                      &instrumentation_desc_set, static_cast<uint32_t>(dynamic_offsets.size()),
                                      dynamic_offsets.data());
    } else {
        // If no pipeline layout was bound when using shader objects that don't use any descriptor set, bind the debug pipeline
        // layout
        DispatchCmdBindDescriptorSets(cmd_buffer->VkHandle(), bind_point, GetDebugPipelineLayout(), desc_set_bind_index, 1,
                                      &instrumentation_desc_set, static_cast<uint32_t>(dynamic_offsets.size()),
                                      dynamic_offsets.data());
    }

    if (pipeline_state && pipeline_layout_handle == VK_NULL_HANDLE) {
        ReportSetupProblem(cmd_buffer->Handle(), loc,
                           "Unable to find pipeline layout to bind debug descriptor set. Aborting GPU-AV");
        aborted = true;
        return CommandResources();
    }

    // It is possible to have no descriptor sets bound, for example if using push constants.
    uint32_t di_buf_index =
        cmd_buffer->di_input_buffer_list.size() > 0 ? uint32_t(cmd_buffer->di_input_buffer_list.size()) - 1 : vvl::kU32Max;

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

CommandResources Validator::AllocateActionCommandResources(VkCommandBuffer cmd_buffer, VkPipelineBindPoint bind_point,
                                                           const Location &loc,
                                                           const CmdIndirectState *indirect_state /*= nullptr*/) {
    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(cmd_buffer, loc, "Unrecognized command buffer");
        aborted = true;
        return CommandResources();
    }
    return AllocateActionCommandResources(cb_node, bind_point, loc, indirect_state);
}

bool Validator::AllocateOutputMem(DeviceMemoryBlock &output_mem, const Location &loc) {
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = output_buffer_byte_size;
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.pool = output_buffer_pool;
    VkResult result = vmaCreateBuffer(vmaAllocator, &buffer_info, &alloc_info, &output_mem.buffer, &output_mem.allocation, nullptr);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Unable to allocate device memory for error output buffer. Device could become unstable.",
                           true);
        aborted = true;
        return false;
    }

    uint32_t *output_buffer_ptr;
    result = vmaMapMemory(vmaAllocator, output_mem.allocation, reinterpret_cast<void **>(&output_buffer_ptr));
    if (result == VK_SUCCESS) {
        memset(output_buffer_ptr, 0, output_buffer_byte_size);
        if (gpuav_settings.validate_descriptors) {
            output_buffer_ptr[cst::stream_output_flags_offset] = cst::inst_buffer_oob_enabled;
        }
        vmaUnmapMemory(vmaAllocator, output_mem.allocation);
    } else {
        ReportSetupProblem(device, loc,
                           "Unable to map device memory allocated for error output buffer. Device could become unstable.", true);
        aborted = true;
        return false;
    }

    return true;
}

// Draw validation resources

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
        BindDiagnosticCallsCommonDescSet(cb_node, VK_PIPELINE_BIND_POINT_GRAPHICS, shared_resources->pipeline_layout,
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
        shader_ci.codeSize = gpu_pre_draw_vert_size * sizeof(uint32_t);
        shader_ci.pCode = gpu_pre_draw_vert;
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
        shader_module_ci.codeSize = gpu_pre_draw_vert_size * sizeof(uint32_t);
        shader_module_ci.pCode = gpu_pre_draw_vert;
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

// Dispatch validation resources

std::unique_ptr<CommandResources> Validator::AllocatePreDispatchIndirectValidationResources(const Location &loc,
                                                                                            VkCommandBuffer cmd_buffer,
                                                                                            VkBuffer indirect_buffer,
                                                                                            VkDeviceSize indirect_offset) {
    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(cmd_buffer, loc, "Unrecognized command buffer");
        aborted = true;
        return nullptr;
    }

    if (!gpuav_settings.validate_indirect_dispatches_buffers) {
        CommandResources cmd_resources = AllocateActionCommandResources(cb_node, VK_PIPELINE_BIND_POINT_COMPUTE, loc);
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
        result = desc_set_manager->GetDescriptorSet(&dispatch_resources->desc_pool, shared_resources->ds_layout,
                                                    &dispatch_resources->indirect_buffer_desc_set);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(cmd_buffer, loc, "Unable to allocate descriptor set. Aborting GPU-AV");
            aborted = true;
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
        BindDiagnosticCallsCommonDescSet(cb_node, VK_PIPELINE_BIND_POINT_COMPUTE, shared_resources->pipeline_layout,
                                         cb_node->compute_index, static_cast<uint32_t>(cb_node->per_command_resources.size()));
        DispatchCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shared_resources->pipeline_layout,
                                      glsl::kDiagPerCmdDescriptorSet, 1, &dispatch_resources->indirect_buffer_desc_set, 0, nullptr);
        DispatchCmdDispatch(cmd_buffer, 1, 1, 1);

        CommandResources cmd_resources = AllocateActionCommandResources(cb_node, VK_PIPELINE_BIND_POINT_COMPUTE, loc);
        if (aborted) {
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
        ReportSetupProblem(device, loc, "Unable to create descriptor set layout. Aborting GPU-AV");
        aborted = true;
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
        ReportSetupProblem(device, loc, "Unable to create pipeline layout. Aborting GPU-AV");
        aborted = true;
        return nullptr;
    }

    if (use_shader_objects) {
        VkShaderCreateInfoEXT shader_ci = vku::InitStructHelper();
        shader_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shader_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        shader_ci.codeSize = gpu_pre_dispatch_comp_size * sizeof(uint32_t);
        shader_ci.pCode = gpu_pre_dispatch_comp;
        shader_ci.pName = "main";
        shader_ci.setLayoutCount = pipeline_layout_ci.setLayoutCount;
        shader_ci.pSetLayouts = pipeline_layout_ci.pSetLayouts;
        shader_ci.pushConstantRangeCount = pipeline_layout_ci.pushConstantRangeCount;
        shader_ci.pPushConstantRanges = pipeline_layout_ci.pPushConstantRanges;
        result = DispatchCreateShadersEXT(device, 1u, &shader_ci, nullptr, &shared_resources->shader_object);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, loc, "Unable to create shader object. Aborting GPU-AV");
            aborted = true;
            return nullptr;
        }
    } else {
        VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
        shader_module_ci.codeSize = gpu_pre_dispatch_comp_size * sizeof(uint32_t);
        shader_module_ci.pCode = gpu_pre_dispatch_comp;
        VkShaderModule validation_shader = VK_NULL_HANDLE;
        result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &validation_shader);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, loc, "Unable to create shader module. Aborting GPU-AV");
            aborted = true;
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
            ReportSetupProblem(device, loc, "Failed to create compute pipeline for pre dispatch validation.");
            return nullptr;
        }
    }

    const auto elt =
        shared_validation_resources_map.insert({typeid(PreDispatchResources::SharedResources), std::move(shared_resources)});

    assert(elt.second);

    return reinterpret_cast<PreDispatchResources::SharedResources *>(elt.first->second.get());
}

// Trace rays validation resources

std::unique_ptr<CommandResources> Validator::AllocatePreTraceRaysValidationResources(const Location &loc,
                                                                                     VkCommandBuffer cmd_buffer,
                                                                                     VkDeviceAddress indirect_data_address) {
    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(cmd_buffer, loc, "Unrecognized command buffer");
        aborted = true;
        return nullptr;
    }

    if (!gpuav_settings.validate_indirect_trace_rays_buffers) {
        CommandResources cmd_resources = AllocateActionCommandResources(cb_node, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, loc);
        auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
        return cmd_resources_ptr;
    }

    auto trace_rays_resources = std::make_unique<PreTraceRaysResources>();

    {
        PreTraceRaysResources::SharedResources *shared_resources =
            GetSharedTraceRaysValidationResources(cb_node->GetValidationCmdCommonDescriptorSetLayout(), loc);
        if (!shared_resources) {
            return nullptr;
        }

        trace_rays_resources->indirect_data_address = indirect_data_address;

        // Save current ray tracing pipeline state
        RestorablePipelineState restorable_state(*cb_node, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

        // Push info needed for validation:
        // - the device address indirect data is read from
        // - the limits to check against
        uint32_t push_constants[PreTraceRaysResources::push_constant_words] = {};
        const uint64_t ray_query_dimension_max_width = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[0]) *
                                                       static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[0]);
        const uint64_t ray_query_dimension_max_height = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[1]) *
                                                        static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[1]);
        const uint64_t ray_query_dimension_max_depth = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[2]) *
                                                       static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[2]);
        // Need to put the buffer reference first otherwise it is incorrect, probably an alignment issue
        push_constants[0] = static_cast<uint32_t>(trace_rays_resources->indirect_data_address) & vvl::kU32Max;
        push_constants[1] = static_cast<uint32_t>(trace_rays_resources->indirect_data_address >> 32) & vvl::kU32Max;
        push_constants[2] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_width, vvl::kU32Max));
        push_constants[3] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_height, vvl::kU32Max));
        push_constants[4] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_depth, vvl::kU32Max));

        DispatchCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, shared_resources->pipeline);
        BindDiagnosticCallsCommonDescSet(cb_node, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, shared_resources->pipeline_layout,
                                         cb_node->trace_rays_index, static_cast<uint32_t>(cb_node->per_command_resources.size()));
        DispatchCmdPushConstants(cmd_buffer, shared_resources->pipeline_layout, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0,
                                 sizeof(push_constants), push_constants);
        VkStridedDeviceAddressRegionKHR ray_gen_sbt{};
        assert(shared_resources->sbt_address != 0);
        ray_gen_sbt.deviceAddress = shared_resources->sbt_address;
        ray_gen_sbt.stride = shared_resources->shader_group_handle_size_aligned;
        ray_gen_sbt.size = shared_resources->shader_group_handle_size_aligned;

        VkStridedDeviceAddressRegionKHR empty_sbt{};
        DispatchCmdTraceRaysKHR(cmd_buffer, &ray_gen_sbt, &empty_sbt, &empty_sbt, &empty_sbt, 1, 1, 1);

        CommandResources cmd_resources = AllocateActionCommandResources(cb_node, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, loc);
        if (aborted) {
            return nullptr;
        }
        CommandResources &base = *trace_rays_resources;
        base = cmd_resources;

        // Restore the previous ray tracing pipeline state.
        restorable_state.Restore(cmd_buffer);
    }

    return trace_rays_resources;
}

PreTraceRaysResources::SharedResources *Validator::GetSharedTraceRaysValidationResources(
    VkDescriptorSetLayout error_output_desc_layout, const Location &loc) {
    if (auto shared_resources = shared_validation_resources_map.find(typeid(PreTraceRaysResources::SharedResources));
        shared_resources != shared_validation_resources_map.end()) {
        return reinterpret_cast<PreTraceRaysResources::SharedResources *>(shared_resources->second.get());
    }

    auto shared_resources = std::make_unique<PreTraceRaysResources::SharedResources>();

    VkResult result = VK_SUCCESS;

    VkPushConstantRange push_constant_range = {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    push_constant_range.offset = 0;
    push_constant_range.size = PreTraceRaysResources::push_constant_words * sizeof(uint32_t);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pushConstantRangeCount = 1;
    pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &error_output_desc_layout;
    result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &shared_resources->pipeline_layout);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Unable to create pipeline layout. Aborting GPU-AV");
        aborted = true;
        return nullptr;
    }

    VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
    shader_module_ci.codeSize = gpu_pre_trace_rays_rgen_size * sizeof(uint32_t);
    shader_module_ci.pCode = gpu_pre_trace_rays_rgen;
    VkShaderModule validation_shader = VK_NULL_HANDLE;
    result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &validation_shader);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Unable to create ray tracing shader module. Aborting GPU-AV");
        aborted = true;
        return nullptr;
    }

    // Create pipeline
    VkPipelineShaderStageCreateInfo pipeline_stage_ci = vku::InitStructHelper();
    pipeline_stage_ci.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    pipeline_stage_ci.module = validation_shader;
    pipeline_stage_ci.pName = "main";
    VkRayTracingShaderGroupCreateInfoKHR raygen_group_ci = vku::InitStructHelper();
    raygen_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    raygen_group_ci.generalShader = 0;
    raygen_group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
    raygen_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
    raygen_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR rt_pipeline_create_info{};
    rt_pipeline_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rt_pipeline_create_info.stageCount = 1;
    rt_pipeline_create_info.pStages = &pipeline_stage_ci;
    rt_pipeline_create_info.groupCount = 1;
    rt_pipeline_create_info.pGroups = &raygen_group_ci;
    rt_pipeline_create_info.maxPipelineRayRecursionDepth = 1;
    rt_pipeline_create_info.layout = shared_resources->pipeline_layout;
    result = DispatchCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rt_pipeline_create_info, nullptr,
                                                  &shared_resources->pipeline);

    DispatchDestroyShaderModule(device, validation_shader, nullptr);

    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Failed to create ray tracing pipeline for pre trace rays validation. Aborting GPU-AV");
        aborted = true;
        return nullptr;
    }

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&rt_pipeline_props);
    DispatchGetPhysicalDeviceProperties2(physical_device, &props2);

    // Get shader group handles to fill shader binding table (SBT)
    const uint32_t shader_group_size_aligned =
        Align(rt_pipeline_props.shaderGroupHandleSize, rt_pipeline_props.shaderGroupHandleAlignment);
    const uint32_t sbt_size = 1 * shader_group_size_aligned;
    std::vector<uint8_t> sbt_host_storage(sbt_size);
    result = DispatchGetRayTracingShaderGroupHandlesKHR(device, shared_resources->pipeline, 0, rt_pipeline_create_info.groupCount,
                                                        sbt_size, sbt_host_storage.data());
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Failed to call vkGetRayTracingShaderGroupHandlesKHR. Aborting GPU-AV");
        aborted = true;
        return nullptr;
    }

    // Allocate buffer to store SBT, and fill it with sbt_host_storage
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = 4096;
    buffer_info.usage =
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    uint32_t mem_type_index = 0;
    vmaFindMemoryTypeIndexForBufferInfo(vmaAllocator, &buffer_info, &alloc_info, &mem_type_index);
    VmaPoolCreateInfo pool_create_info = {};
    pool_create_info.memoryTypeIndex = mem_type_index;
    pool_create_info.blockSize = 0;
    pool_create_info.maxBlockCount = 0;
    pool_create_info.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
    result = vmaCreatePool(vmaAllocator, &pool_create_info, &shared_resources->sbt_pool);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Unable to create VMA memory pool for SBT. Aborting GPU-AV");
        aborted = true;
        return nullptr;
    }

    alloc_info.pool = shared_resources->sbt_pool;
    result = vmaCreateBuffer(vmaAllocator, &buffer_info, &alloc_info, &shared_resources->sbt_buffer,
                             &shared_resources->sbt_allocation, nullptr);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Unable to allocate device memory for shader binding table. Aborting GPU-AV.", true);
        aborted = true;
        return nullptr;
    }

    uint8_t *mapped_sbt = nullptr;
    result = vmaMapMemory(vmaAllocator, shared_resources->sbt_allocation, reinterpret_cast<void **>(&mapped_sbt));

    if (result != VK_SUCCESS) {
        ReportSetupProblem(
            device, loc, "Failed to map shader binding table when creating trace rays validation resources. Aborting GPU-AV", true);
        aborted = true;
        return nullptr;
    }

    std::memcpy(mapped_sbt, sbt_host_storage.data(), rt_pipeline_props.shaderGroupHandleSize);

    vmaUnmapMemory(vmaAllocator, shared_resources->sbt_allocation);

    shared_resources->shader_group_handle_size_aligned = shader_group_size_aligned;

    // Retrieve SBT address
    const VkDeviceAddress sbt_address = GetBufferDeviceAddress(shared_resources->sbt_buffer, loc);
    assert(sbt_address != 0);
    if (sbt_address == 0) {
        ReportSetupProblem(device, loc, "Retrieved SBT buffer device address is null. Aborting GPU-AV.");
        aborted = true;
        return nullptr;
    }
    assert(sbt_address == Align(sbt_address, static_cast<VkDeviceAddress>(rt_pipeline_props.shaderGroupBaseAlignment)));
    shared_resources->sbt_address = sbt_address;

    const auto elt =
        shared_validation_resources_map.insert({typeid(PreTraceRaysResources::SharedResources), std::move(shared_resources)});

    assert(elt.second);

    return reinterpret_cast<PreTraceRaysResources::SharedResources *>(elt.first->second.get());
}

// Copy buffer to image validation resources

std::unique_ptr<CommandResources> Validator::AllocatePreCopyBufferToImageValidationResources(
    const Location &loc, VkCommandBuffer cmd_buffer, const VkCopyBufferToImageInfo2 *copy_buffer_to_img_info) {
    if (aborted) {
        return nullptr;
    }

    if (!gpuav_settings.validate_buffer_copies) {
        return nullptr;
    }

    // No need to perform validation if VK_EXT_depth_range_unrestricted is enabled
    if (IsExtEnabled(device_extensions.vk_ext_depth_range_unrestricted)) {
        return nullptr;
    }

    auto image_state = Get<vvl::Image>(copy_buffer_to_img_info->dstImage);
    if (!image_state) {
        ReportSetupProblem(cmd_buffer, loc, "AllocatePreCopyBufferToImageValidationResources: Unrecognized image");
        aborted = true;
        return nullptr;
    }

    // Only need to perform validation for depth image having a depth format that is not unsigned normalized.
    // For unsigned normalized formats, depth is by definition in range [0, 1]
    if (!IsValueIn(image_state->create_info.format, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT})) {
        return nullptr;
    }

    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(cmd_buffer, loc, "AllocatePreCopyBufferToImageValidationResources: Unrecognized command buffer");
        aborted = true;
        return nullptr;
    }

    PreCopyBufferToImageResources::SharedResources *shared_resources =
        GetSharedCopyBufferToImageValidationResources(cb_node->GetValidationCmdCommonDescriptorSetLayout(), loc);
    if (!shared_resources) {
        return nullptr;
    }

    CommandResources cmd_resources;
    cmd_resources.command = loc.function;

    auto copy_buffer_to_img_resources = std::make_unique<PreCopyBufferToImageResources>();
    CommandResources &base = *copy_buffer_to_img_resources;
    base = cmd_resources;
    copy_buffer_to_img_resources->src_buffer = copy_buffer_to_img_info->srcBuffer;

    // Allocate buffer that will be used to store pRegions
    uint32_t max_texels_count_in_regions = copy_buffer_to_img_info->pRegions[0].imageExtent.width *
                                           copy_buffer_to_img_info->pRegions[0].imageExtent.height *
                                           copy_buffer_to_img_info->pRegions[0].imageExtent.depth;
    {
        // Needs to be kept in sync with gpu_pre_copy_buffer_to_image.comp
        struct BufferImageCopy {
            uint32_t src_buffer_byte_offset;
            uint32_t start_layer;
            uint32_t layer_count;
            uint32_t row_extent;
            uint32_t slice_extent;
            uint32_t layer_extent;
            uint32_t pad_[2];
            int32_t image_offset[4];
            uint32_t image_extent[4];
        };

        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        const VkDeviceSize uniform_block_constants_byte_size = (4 +  // image extent
                                                                1 +  // block size
                                                                1 +  // gpu copy regions count
                                                                2    // pad
                                                                ) *
                                                               sizeof(uint32_t);
        buffer_info.size = uniform_block_constants_byte_size + sizeof(BufferImageCopy) * copy_buffer_to_img_info->regionCount;
        buffer_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        uint32_t mem_type_index = 0;
        vmaFindMemoryTypeIndexForBufferInfo(vmaAllocator, &buffer_info, &alloc_info, &mem_type_index);

        alloc_info.pool = shared_resources->copy_regions_pool;
        VkResult result =
            vmaCreateBuffer(vmaAllocator, &buffer_info, &alloc_info, &copy_buffer_to_img_resources->copy_src_regions_buffer,
                            &copy_buffer_to_img_resources->copy_src_regions_allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(cmd_buffer, loc, "Unable to allocate device memory for GPU copy of pRegions. Aborting GPU-AV.",
                               true);
            aborted = true;
            return nullptr;
        }

        uint32_t *gpu_regions_u32_ptr = nullptr;
        result = vmaMapMemory(vmaAllocator, copy_buffer_to_img_resources->copy_src_regions_allocation,
                              reinterpret_cast<void **>(&gpu_regions_u32_ptr));

        if (result != VK_SUCCESS) {
            ReportSetupProblem(cmd_buffer, loc, "Unable to map device memory for GPU copy of pRegions. Aborting GPU-AV.", true);
            aborted = true;
            return nullptr;
        }

        const uint32_t block_size = image_state->create_info.format == VK_FORMAT_D32_SFLOAT ? 4 : 5;
        uint32_t gpu_regions_count = 0;
        BufferImageCopy *gpu_regions_ptr =
            reinterpret_cast<BufferImageCopy *>(&gpu_regions_u32_ptr[uniform_block_constants_byte_size / sizeof(uint32_t)]);
        for (const auto &cpu_region : vvl::make_span(copy_buffer_to_img_info->pRegions, copy_buffer_to_img_info->regionCount)) {
            if (cpu_region.imageSubresource.aspectMask != VK_IMAGE_ASPECT_DEPTH_BIT) {
                continue;
            }

            // Read offset above kU32Max cannot be indexed in the validation shader
            if (const VkDeviceSize max_buffer_read_offset =
                    cpu_region.bufferOffset + static_cast<VkDeviceSize>(block_size) * cpu_region.imageExtent.width *
                                                  cpu_region.imageExtent.height * cpu_region.imageExtent.depth;
                max_buffer_read_offset > static_cast<VkDeviceSize>(vvl::kU32Max)) {
                continue;
            }

            BufferImageCopy &gpu_region = gpu_regions_ptr[gpu_regions_count];
            gpu_region.src_buffer_byte_offset = static_cast<uint32_t>(cpu_region.bufferOffset);
            gpu_region.start_layer = cpu_region.imageSubresource.baseArrayLayer;
            gpu_region.layer_count = cpu_region.imageSubresource.layerCount;
            gpu_region.row_extent = std::max(cpu_region.bufferRowLength, image_state->create_info.extent.width * block_size);
            gpu_region.slice_extent =
                std::max(cpu_region.bufferImageHeight, image_state->create_info.extent.height * gpu_region.row_extent);
            gpu_region.layer_extent = image_state->create_info.extent.depth * gpu_region.slice_extent;
            gpu_region.image_offset[0] = cpu_region.imageOffset.x;
            gpu_region.image_offset[1] = cpu_region.imageOffset.y;
            gpu_region.image_offset[2] = cpu_region.imageOffset.z;
            gpu_region.image_offset[3] = 0;
            gpu_region.image_extent[0] = cpu_region.imageExtent.width;
            gpu_region.image_extent[1] = cpu_region.imageExtent.height;
            gpu_region.image_extent[2] = cpu_region.imageExtent.depth;
            gpu_region.image_extent[3] = 0;

            max_texels_count_in_regions =
                std::max(max_texels_count_in_regions,
                         cpu_region.imageExtent.width * cpu_region.imageExtent.height * cpu_region.imageExtent.depth);

            ++gpu_regions_count;
        }

        if (gpu_regions_count == 0) {
            // Nothing to validate
            vmaUnmapMemory(vmaAllocator, copy_buffer_to_img_resources->copy_src_regions_allocation);
            copy_buffer_to_img_resources->Destroy(*this);
            return nullptr;
        }

        gpu_regions_u32_ptr[0] = image_state->create_info.extent.width;
        gpu_regions_u32_ptr[1] = image_state->create_info.extent.height;
        gpu_regions_u32_ptr[2] = image_state->create_info.extent.depth;
        gpu_regions_u32_ptr[3] = 0;
        gpu_regions_u32_ptr[4] = block_size;
        gpu_regions_u32_ptr[5] = gpu_regions_count;
        gpu_regions_u32_ptr[6] = 0;
        gpu_regions_u32_ptr[7] = 0;

        vmaUnmapMemory(vmaAllocator, copy_buffer_to_img_resources->copy_src_regions_allocation);
    }

    // Update descriptor set
    {
        VkResult result = desc_set_manager->GetDescriptorSet(&copy_buffer_to_img_resources->desc_pool, shared_resources->ds_layout,
                                                             &copy_buffer_to_img_resources->desc_set);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(cmd_buffer, loc,
                               "Unable to allocate descriptor set for copy buffer to image validation. Aborting GPU-AV");
            aborted = true;
            return nullptr;
        }

        std::array<VkDescriptorBufferInfo, 2> descriptor_buffer_infos = {};
        // Copy source buffer
        descriptor_buffer_infos[0].buffer = copy_buffer_to_img_info->srcBuffer;
        descriptor_buffer_infos[0].offset = 0;
        descriptor_buffer_infos[0].range = VK_WHOLE_SIZE;
        // Copy regions buffer
        descriptor_buffer_infos[1].buffer = copy_buffer_to_img_resources->copy_src_regions_buffer;
        descriptor_buffer_infos[1].offset = 0;
        descriptor_buffer_infos[1].range = VK_WHOLE_SIZE;

        std::array<VkWriteDescriptorSet, descriptor_buffer_infos.size()> desc_writes = {};
        for (const auto [i, desc_buffer_info] : vvl::enumerate(descriptor_buffer_infos.data(), descriptor_buffer_infos.size())) {
            desc_writes[i] = vku::InitStructHelper();
            desc_writes[i].dstBinding = static_cast<uint32_t>(i);
            desc_writes[i].descriptorCount = 1;
            desc_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            desc_writes[i].pBufferInfo = desc_buffer_info;
            desc_writes[i].dstSet = copy_buffer_to_img_resources->desc_set;
        }
        DispatchUpdateDescriptorSets(device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, nullptr);
    }
    // Save current graphics pipeline state
    RestorablePipelineState restorable_state(*cb_node, VK_PIPELINE_BIND_POINT_COMPUTE);

    // Insert diagnostic dispatch
    DispatchCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shared_resources->pipeline);

    BindDiagnosticCallsCommonDescSet(cb_node, VK_PIPELINE_BIND_POINT_COMPUTE, shared_resources->pipeline_layout, 0,
                                     static_cast<uint32_t>(cb_node->per_command_resources.size()));
    DispatchCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shared_resources->pipeline_layout,
                                  glsl::kDiagPerCmdDescriptorSet, 1, &copy_buffer_to_img_resources->desc_set, 0, nullptr);
    // correct_count == max texelsCount?
    const uint32_t group_count_x = max_texels_count_in_regions / 64 + uint32_t(max_texels_count_in_regions % 64 > 0);
    DispatchCmdDispatch(cmd_buffer, group_count_x, 1, 1);

    // Restore the previous compute pipeline state.
    restorable_state.Restore(cmd_buffer);

    return copy_buffer_to_img_resources;
}

PreCopyBufferToImageResources::SharedResources *Validator::GetSharedCopyBufferToImageValidationResources(
    VkDescriptorSetLayout error_output_set_layout, const Location &loc) {
    if (auto shared_resources = shared_validation_resources_map.find(typeid(PreCopyBufferToImageResources::SharedResources));
        shared_resources != shared_validation_resources_map.end()) {
        return reinterpret_cast<PreCopyBufferToImageResources::SharedResources *>(shared_resources->second.get());
    }

    auto shared_resources = std::make_unique<PreCopyBufferToImageResources::SharedResources>();

    VkResult result = VK_SUCCESS;
    const std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},  // copy source buffer
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},  // copy regions buffer
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

    // Allocate buffer memory pool that will be used to create the buffers holding copy regions
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = 4096;  // Dummy value
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    uint32_t mem_type_index = 0;
    vmaFindMemoryTypeIndexForBufferInfo(vmaAllocator, &buffer_info, &alloc_info, &mem_type_index);
    VmaPoolCreateInfo pool_create_info = {};
    pool_create_info.memoryTypeIndex = mem_type_index;
    pool_create_info.blockSize = 0;
    pool_create_info.maxBlockCount = 0;
    pool_create_info.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
    result = vmaCreatePool(vmaAllocator, &pool_create_info, &shared_resources->copy_regions_pool);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Unable to create VMA memory pool for buffer to image copies validation. Aborting GPU-AV");
        aborted = true;
        return nullptr;
    }

    std::array<VkDescriptorSetLayout, 2> set_layouts = {{error_output_set_layout, shared_resources->ds_layout}};
    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
    pipeline_layout_ci.pSetLayouts = set_layouts.data();
    result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &shared_resources->pipeline_layout);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Unable to create pipeline layout. Aborting GPU-AV");
        aborted = true;
        return nullptr;
    }

    VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
    shader_module_ci.codeSize = gpu_pre_copy_buffer_to_image_comp_size * sizeof(uint32_t);
    shader_module_ci.pCode = gpu_pre_copy_buffer_to_image_comp;
    VkShaderModule validation_shader = VK_NULL_HANDLE;
    result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &validation_shader);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Unable to create shader module. Aborting GPU-AV");
        aborted = true;
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
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, loc, "Failed to create compute pipeline for copy buffer to image validation. Aborting GPU-AV.");
    }

    DispatchDestroyShaderModule(device, validation_shader, nullptr);

    const auto elt = shared_validation_resources_map.insert(
        {typeid(PreCopyBufferToImageResources::SharedResources), std::move(shared_resources)});

    assert(elt.second);

    return reinterpret_cast<PreCopyBufferToImageResources::SharedResources *>(elt.first->second.get());
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

void Validator::StoreCommandResources(const VkCommandBuffer cmd_buffer, std::unique_ptr<CommandResources> command_resources,
                                      const Location &loc) {
    if (aborted) return;
    if (!command_resources) return;

    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(cmd_buffer, loc, "Unrecognized command buffer");
        aborted = true;
        return;
    }

    cb_node->per_command_resources.emplace_back(std::move(command_resources));
}

}  // namespace gpuav
