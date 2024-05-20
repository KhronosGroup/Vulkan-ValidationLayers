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
#include "gpu/core/gpuav_constants.h"
#include "gpu/core/gpuav.h"
#include "gpu/resources/gpuav_subclasses.h"
#include "gpu/spirv/module.h"
#include "spirv-tools/instrument.hpp"
#include "spirv-tools/linker.hpp"
#include "generated/layer_chassis_dispatch.h"
#include "gpu/error_message/gpuav_vuids.h"
#include "containers/custom_containers.h"
#include "chassis/chassis_modification_state.h"
#include "state_tracker/render_pass_state.h"

namespace gpuav {

VkDeviceAddress Validator::GetBufferDeviceAddress(VkBuffer buffer, const Location &loc) const {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8001
    // Setting enabled_features.bufferDeviceAddress to true in GpuShaderInstrumentor::PreCallRecordCreateDevice
    // when adding missing features will modify another validator object, one associated to VkInstance,
    // and "this" validator is associated to a device. enabled_features is not inherited, and besides
    // would be reset in GetEnabledDeviceFeatures.
    // The switch from the instance validator object to the device one happens in
    // `state_tracker.cpp`, `ValidationStateTracker::PostCallRecordCreateDevice`
    // TL;DR is the following type of sanity check is currently invalid, but it would be nice to have
    // assert(enabled_features.bufferDeviceAddress);

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
        if (!dce_pass.Run(instrumented_spirv.data(), instrumented_spirv.size(), &instrumented_spirv, opt_options)) {
            InternalError(device, loc,
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
    if (!gpuav_settings.validate_descriptors) return;

    auto cb_node = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_node) {
        InternalError(commandBuffer, loc, "Unrecognized command buffer");
        return;
    }
    const auto lv_bind_point = ConvertToLvlBindPoint(pipelineBindPoint);
    auto const &last_bound = cb_node->lastBound[lv_bind_point];
    // Should have just been updated
    if (!last_bound.pipeline_state) {
        InternalError(pipeline, loc, "Unrecognized pipeline");
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
    auto cb_node = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_node) {
        InternalError(commandBuffer, loc, "Unrecognized command buffer");
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
            InternalError(commandBuffer, loc, "Unable to allocate device memory. Device could become unstable.", true);
            return;
        }
        glsl::BindlessStateBuffer *bindless_state{nullptr};
        result =
            vmaMapMemory(vmaAllocator, di_buffers.bindless_state_buffer_allocation, reinterpret_cast<void **>(&bindless_state));
        if (result != VK_SUCCESS) {
            InternalError(commandBuffer, loc, "Unable to map device memory. Device could become unstable.", true);
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

void Validator::BindValidationCmdsCommonDescSet(const LockedSharedPtr<CommandBuffer, WriteLockGuard> &cmd_buffer_state,
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

    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    auto const &last_bound = cmd_buffer->lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;

    if (!pipeline_state && !last_bound.HasShaderObjects()) {
        InternalError(cmd_buffer->VkHandle(), loc, "Neither pipeline state nor shader object states were found, aborting GPU-AV");
        return CommandResources();
    }

    VkDescriptorSet instrumentation_desc_set = VK_NULL_HANDLE;
    VkDescriptorPool instrumentation_desc_pool = VK_NULL_HANDLE;
    VkResult result = desc_set_manager->GetDescriptorSet(
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

CommandResources Validator::AllocateActionCommandResources(VkCommandBuffer cmd_buffer, VkPipelineBindPoint bind_point,
                                                           const Location &loc,
                                                           const CmdIndirectState *indirect_state /*= nullptr*/) {
    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        InternalError(cmd_buffer, loc, "Unrecognized command buffer");
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
        InternalError(device, loc, "Unable to allocate device memory for error output buffer. Device could become unstable.", true);
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
        InternalError(device, loc, "Unable to map device memory allocated for error output buffer. Device could become unstable.",
                      true);
        return false;
    }

    return true;
}

void Validator::StoreCommandResources(const VkCommandBuffer cmd_buffer, std::unique_ptr<CommandResources> command_resources,
                                      const Location &loc) {
    if (!command_resources) return;

    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        InternalError(cmd_buffer, loc, "Unrecognized command buffer");
        return;
    }

    cb_node->per_command_resources.emplace_back(std::move(command_resources));
}

}  // namespace gpuav
