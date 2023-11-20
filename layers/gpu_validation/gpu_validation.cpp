/* Copyright (c) 2018-2023 The Khronos Group Inc.
 * Copyright (c) 2018-2023 Valve Corporation
 * Copyright (c) 2018-2023 LunarG, Inc.
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
#include "gpu_validation/gpu_validation.h"
#include "spirv-tools/instrument.hpp"
#include "spirv-tools/linker.hpp"
#include "generated/layer_chassis_dispatch.h"
#include "gpu_vuids.h"
#include "containers/custom_containers.h"
// Generated shaders
#include "generated/gpu_pre_draw_vert.h"
#include "generated/gpu_pre_dispatch_comp.h"
#include "generated/gpu_pre_trace_rays_rgen.h"
#include "generated/gpu_as_inspection_comp.h"
#include "generated/inst_functions_comp.h"
#include "generated/gpu_inst_shader_hash.h"

VkDeviceAddress gpuav::Validator::GetBufferDeviceAddress(VkBuffer buffer) const {
    if (!buffer_device_address_enabled) {
        assert(false);
        ReportSetupProblem(device, "Buffer device address feature not enabled, calling GetBufferDeviceAddress is invalid");
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

bool gpuav::Validator::CheckForDescriptorIndexing(DeviceFeatures enabled_features) const {
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
                              std::string &error) {
    // Use SPIRV-Tools validator to try and catch any issues with the module
    spv_target_env spirv_environment = SPV_ENV_VULKAN_1_1;
    spv_context ctx = spvContextCreate(spirv_environment);
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
bool gpuav::Validator::InstrumentShader(const vvl::span<const uint32_t> &input, std::vector<uint32_t> &new_pgm,
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

    // Call the optimizer to instrument the shader.
    // Use the unique_shader_module_id as a shader ID so we can look up its handle later in the shader_map.
    // If descriptor indexing is enabled, enable length checks and updated descriptor checks
    using namespace spvtools;
    spv_target_env target_env = PickSpirvEnv(api_version, IsExtEnabled(device_extensions.vk_khr_spirv_1_4));

    // Instrument the user's shader
    {
        ValidatorOptions val_options;
        AdjustValidatorOptions(device_extensions, enabled_features, val_options);
        OptimizerOptions opt_options;
        opt_options.set_run_validator(true);
        opt_options.set_validator_options(val_options);
        Optimizer inst_passes(target_env);
        inst_passes.SetMessageConsumer(gpu_console_message_consumer);
        if (gpuav_settings.validate_descriptors) {
            inst_passes.RegisterPass(CreateInstBindlessCheckPass(unique_shader_id));
        }

        if ((IsExtEnabled(device_extensions.vk_ext_buffer_device_address) ||
             IsExtEnabled(device_extensions.vk_khr_buffer_device_address)) &&
            shaderInt64 && enabled_features.bufferDeviceAddress) {
            inst_passes.RegisterPass(CreateInstBuffAddrCheckPass(unique_shader_id));
        }
        if (!inst_passes.Run(binaries[0].data(), binaries[0].size(), &binaries[0], opt_options)) {
            ReportSetupProblem(device, "Failure to instrument shader.  Proceeding with non-instrumented shader.");
            assert(false);
            return false;
        }
    }
    {
        // The instrumentation code is not a complete SPIRV module so we cannot validate it separately
        OptimizerOptions options;
        options.set_run_validator(false);
        // Load instrumentation helper functions
        size_t inst_size = sizeof(inst_functions_comp) / sizeof(uint32_t);
        binaries[1].reserve(inst_size);  // the shader will be copied in by the optimizer

        // The compiled instrumentation functions use 7 for their data.
        // Switch that to the highest set number supported by the actual VkDevice.
        Optimizer switch_descriptorsets(target_env);
        switch_descriptorsets.SetMessageConsumer(gpu_console_message_consumer);
        switch_descriptorsets.RegisterPass(CreateSwitchDescriptorSetPass(7, desc_set_bind_index));

        if (!switch_descriptorsets.Run(inst_functions_comp, inst_size, &binaries[1], options)) {
            ReportSetupProblem(
                device, "Failure to switch descriptorsets in instrumentation code. Proceeding with non-instrumented shader.");
            assert(false);
            return false;
        }
    }
    // Link in the instrumentation helper functions
    {
        Context context(target_env);
        context.SetMessageConsumer(gpu_console_message_consumer);
        LinkerOptions link_options;
        link_options.SetUseHighestVersion(true);

        spv_result_t link_status = Link(context, binaries, &new_pgm, link_options);
        if (link_status != SPV_SUCCESS && link_status != SPV_WARNING) {
            std::ostringstream strm;
            strm << "Failed to link Instrumented shader, error = " << link_status << " Proceeding with non instrumented shader.";
            ReportSetupProblem(device, strm.str().c_str());
            assert(false);
            return false;
        }
    }
    // (Maybe) validate the instrumented and linked shader
    if (validate_instrumented_shaders) {
        std::string instrumented_error;
        if (!GpuValidateShader(new_pgm, device_extensions.vk_khr_relaxed_block_layout, device_extensions.vk_ext_scalar_block_layout,
                               instrumented_error)) {
            std::ostringstream strm;
            strm << "Instrumented shader is invalid, error = " << instrumented_error << " Proceeding with non instrumented shader.";
            ReportSetupProblem(device, strm.str().c_str());
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
        if (!dce_pass.Run(new_pgm.data(), new_pgm.size(), &new_pgm, opt_options)) {
            ReportSetupProblem(device, "Failure to run DCE on instrumented shader.  Proceeding with non-instrumented shader.");
            assert(false);
            return false;
        }
    }
    return true;
}

bool gpuav::Validator::CheckForCachedInstrumentedShader(uint32_t shader_hash, create_shader_module_api_state *csm_state) {
    auto it = instrumented_shaders.find(shader_hash);
    if (it != instrumented_shaders.end()) {
        csm_state->instrumented_create_info.codeSize = it->second.first * sizeof(uint32_t);
        csm_state->instrumented_create_info.pCode = it->second.second.data();
        csm_state->instrumented_spirv = it->second.second;
        csm_state->unique_shader_id = shader_hash;
        return true;
    }
    return false;
}

bool gpuav::Validator::CheckForCachedInstrumentedShader(uint32_t index, uint32_t shader_hash,
                                                        create_shader_object_api_state *cso_state) {
    auto it = instrumented_shaders.find(shader_hash);
    if (it != instrumented_shaders.end()) {
        cso_state->instrumented_create_info[index].codeSize = it->second.first * sizeof(uint32_t);
        cso_state->instrumented_create_info[index].pCode = it->second.second.data();
        return true;
    }
    return false;
}

// For the given command buffer, map its debug data buffers and update the status of any update after bind descriptors
void gpuav::Validator::UpdateInstrumentationBuffer(CommandBuffer *cb_node) {
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

void gpuav::Validator::UpdateBDABuffer(DeviceMemoryBlock device_address_buffer) {
    if (gpuav_bda_buffer_version == buffer_device_address_ranges_version) {
        return;
    }
    auto address_ranges = GetBufferAddressRanges();
    auto address_ranges_num_addresses = address_ranges.size();
    if (address_ranges_num_addresses == 0) return;

    // Example BDA input buffer assuming 2 buffers using BDA:
    // Word 0 | Index of start of buffer sizes (in this case 5)
    // Word 1 | 0x0000000000000000
    // Word 2 | Device Address of first buffer  (Addresses sorted in ascending order)
    // Word 3 | Device Address of second buffer
    // Word 4 | 0xffffffffffffffff
    // Word 5 | 0 (size of pretend buffer at word 1)
    // Word 6 | Size in bytes of first buffer
    // Word 7 | Size in bytes of second buffer
    // Word 8 | 0 (size of pretend buffer in word 4)

    uint64_t *bda_data;
    // Make sure to limit writes to size of the buffer
    [[maybe_unused]] VkResult result;
    result = vmaMapMemory(vmaAllocator, device_address_buffer.allocation, reinterpret_cast<void **>(&bda_data));
    assert(result == VK_SUCCESS);
    uint32_t address_index = 1;
    size_t size_index = 3 + address_ranges.size();
    memset(bda_data, 0, static_cast<size_t>(app_bda_buffer_size));
    bda_data[0] = size_index;       // Start of buffer sizes
    bda_data[address_index++] = 0;  // NULL address
    bda_data[size_index++] = 0;
    if (address_ranges_num_addresses > gpuav_settings.gpuav_max_buffer_device_addresses) {
        std::ostringstream problem_string;
        problem_string << "Number of buffer device addresses in use (" << address_ranges_num_addresses
                       << ") is greater than khronos_validation.gpuav_max_buffer_device_addresses ("
                       << gpuav_settings.gpuav_max_buffer_device_addresses
                       << "). Truncating BDA table which could result in invalid validation";
        ReportSetupProblem(device, problem_string.str().c_str());
    }
    size_t num_addresses =
        std::min(static_cast<uint32_t>(address_ranges_num_addresses), gpuav_settings.gpuav_max_buffer_device_addresses);
    for (size_t i = 0; i < num_addresses; i++) {
        bda_data[address_index++] = address_ranges[i].begin;
        bda_data[size_index++] = address_ranges[i].end - address_ranges[i].begin;
    }
    bda_data[address_index] = std::numeric_limits<uintptr_t>::max();
    bda_data[size_index] = 0;
    // Flush the BDA buffer before unmapping so that the new state is visible to the GPU
    result = vmaFlushAllocation(vmaAllocator, device_address_buffer.allocation, 0, VK_WHOLE_SIZE);
    // No good way to handle this error, we should still try to unmap.
    assert(result == VK_SUCCESS);
    vmaUnmapMemory(vmaAllocator, device_address_buffer.allocation);
    gpuav_bda_buffer_version = buffer_device_address_ranges_version;
}

void gpuav::Validator::UpdateBoundDescriptors(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint) {
    if (aborted) return;
    auto cb_node = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_node) {
        ReportSetupProblem(device, "Unrecognized command buffer");
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
            ReportSetupProblem(device, "Unable to allocate device memory. Device could become unstable.", true);
            aborted = true;
            return;
        }
        glsl::BindlessStateBuffer *bindless_state{nullptr};
        result =
            vmaMapMemory(vmaAllocator, di_buffers.bindless_state_buffer_allocation, reinterpret_cast<void **>(&bindless_state));
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to map device memory. Device could become unstable.", true);
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

// This function will add the returned VkPipeline handle to another object incharge of destroying it. Caller does NOT have to
// destroy it
VkPipeline gpuav::Validator::GetDrawValidationPipeline(VkRenderPass render_pass) {
    VkPipeline validation_pipeline = VK_NULL_HANDLE;
    // NOTE: for dynamic rendering, render_pass will be VK_NULL_HANDLE but we'll use that as a map
    // key anyways;

    if (auto pipeline_entry = common_draw_resources.renderpass_to_pipeline.find(render_pass);
        pipeline_entry != common_draw_resources.renderpass_to_pipeline.end()) {
        validation_pipeline = pipeline_entry->second;
    }
    if (validation_pipeline != VK_NULL_HANDLE) {
        return validation_pipeline;
    }
    VkPipelineShaderStageCreateInfo pipeline_stage_ci = vku::InitStructHelper();
    pipeline_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    pipeline_stage_ci.module = common_draw_resources.shader_module;
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
    pipeline_ci.layout = common_draw_resources.pipeline_layout;
    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &pipeline_stage_ci;

    VkResult result = DispatchCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &validation_pipeline);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to create graphics pipeline. Aborting GPU-AV");
        aborted = true;
        return VK_NULL_HANDLE;
    }

    common_draw_resources.renderpass_to_pipeline.insert(render_pass, validation_pipeline);
    return validation_pipeline;
}

void gpuav::Validator::AllocatePreDrawValidationResources(const DeviceMemoryBlock &output_block, const VkRenderPass render_pass,
                                                          const bool use_shader_objects, VkPipeline *pPipeline,
                                                          const CmdIndirectState *indirect_state,
                                                          PreDrawResources &out_draw_resources) {
    VkResult result;
    if (!common_draw_resources.initialized) {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},  // output buffer
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},  // count/draws buffer
        };

        VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
        ds_layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
        ds_layout_ci.pBindings = bindings.data();
        result = DispatchCreateDescriptorSetLayout(device, &ds_layout_ci, nullptr, &common_draw_resources.ds_layout);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to create descriptor set layout. Aborting GPU-AV");
            aborted = true;
            return;
        }

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = out_draw_resources.push_constant_words * sizeof(uint32_t);
        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        pipeline_layout_ci.pushConstantRangeCount = 1;
        pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &common_draw_resources.ds_layout;
        result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &common_draw_resources.pipeline_layout);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to create pipeline layout. Aborting GPU-AV");
            aborted = true;
            return;
        }

        if (use_shader_objects) {
            VkShaderCreateInfoEXT shader_ci = vku::InitStructHelper();
            shader_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shader_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
            shader_ci.codeSize = sizeof(gpu_pre_draw_vert);
            shader_ci.pCode = gpu_pre_draw_vert;
            shader_ci.pName = "main";
            shader_ci.setLayoutCount = 1u;
            shader_ci.pSetLayouts = &common_draw_resources.ds_layout;
            shader_ci.pushConstantRangeCount = 1u;
            shader_ci.pPushConstantRanges = &push_constant_range;
            result = DispatchCreateShadersEXT(device, 1u, &shader_ci, nullptr, &common_draw_resources.shader_object);
            if (result != VK_SUCCESS) {
                ReportSetupProblem(device, "Unable to create shader object. Aborting GPU-AV");
                aborted = true;
                return;
            }
        } else {
            VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
            shader_module_ci.codeSize = sizeof(gpu_pre_draw_vert);
            shader_module_ci.pCode = gpu_pre_draw_vert;
            result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &common_draw_resources.shader_module);
            if (result != VK_SUCCESS) {
                ReportSetupProblem(device, "Unable to create shader module. Aborting GPU-AV");
                aborted = true;
                return;
            }
        }

        common_draw_resources.initialized = true;
    }

    if (!use_shader_objects) {
        *pPipeline = GetDrawValidationPipeline(render_pass);
        if (*pPipeline == VK_NULL_HANDLE) {
            ReportSetupProblem(device, "Could not find or create a pipeline. Aborting GPU-AV");
            aborted = true;
            return;
        }
    }

    result = desc_set_manager->GetDescriptorSet(&out_draw_resources.desc_pool, common_draw_resources.ds_layout,
                                                &out_draw_resources.desc_set);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate descriptor set. Aborting GPU-AV");
        aborted = true;
        return;
    }

    const uint32_t buffer_count = 2;
    VkDescriptorBufferInfo buffer_infos[buffer_count] = {};
    // Error output buffer
    buffer_infos[0].buffer = output_block.buffer;
    buffer_infos[0].offset = 0;
    buffer_infos[0].range = VK_WHOLE_SIZE;
    if (indirect_state->count_buffer) {
        // Count buffer
        buffer_infos[1].buffer = indirect_state->count_buffer;
    } else {
        // Draw Buffer
        buffer_infos[1].buffer = indirect_state->buffer;
    }
    buffer_infos[1].offset = 0;
    buffer_infos[1].range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet desc_writes[buffer_count] = {};
    for (uint32_t i = 0; i < buffer_count; i++) {
        desc_writes[i] = vku::InitStructHelper();
        desc_writes[i].dstBinding = i;
        desc_writes[i].descriptorCount = 1;
        desc_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[i].pBufferInfo = &buffer_infos[i];
        desc_writes[i].dstSet = out_draw_resources.desc_set;
    }
    DispatchUpdateDescriptorSets(device, buffer_count, desc_writes, 0, NULL);
}

void gpuav::Validator::AllocatePreDispatchValidationResources(const DeviceMemoryBlock &output_block,
                                                              const CmdIndirectState *indirect_state, const bool use_shader_objects,
                                                              PreDispatchResources &out_dispatch_resources) {
    VkResult result;
    if (!common_dispatch_resources.initialized) {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},  // output buffer
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},  // indirect buffer
        };

        VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
        ds_layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
        ds_layout_ci.pBindings = bindings.data();
        result = DispatchCreateDescriptorSetLayout(device, &ds_layout_ci, nullptr, &common_dispatch_resources.ds_layout);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to create descriptor set layout. Aborting GPU-AV");
            aborted = true;
            return;
        }

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = out_dispatch_resources.push_constant_words * sizeof(uint32_t);
        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        pipeline_layout_ci.pushConstantRangeCount = 1;
        pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &common_dispatch_resources.ds_layout;
        result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &common_dispatch_resources.pipeline_layout);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to create pipeline layout. Aborting GPU-AV");
            aborted = true;
            return;
        }

        if (use_shader_objects) {
            VkShaderCreateInfoEXT shader_ci = vku::InitStructHelper();
            shader_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shader_ci.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
            shader_ci.codeSize = sizeof(gpu_pre_dispatch_comp);
            shader_ci.pCode = gpu_pre_dispatch_comp;
            shader_ci.pName = "main";
            shader_ci.setLayoutCount = 1u;
            shader_ci.pSetLayouts = &common_dispatch_resources.ds_layout;
            shader_ci.pushConstantRangeCount = 1u;
            shader_ci.pPushConstantRanges = &push_constant_range;
            result = DispatchCreateShadersEXT(device, 1u, &shader_ci, nullptr, &common_dispatch_resources.shader_object);
            if (result != VK_SUCCESS) {
                ReportSetupProblem(device, "Unable to create shader object. Aborting GPU-AV");
                aborted = true;
                return;
            }
        } else {
            VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
            shader_module_ci.codeSize = sizeof(gpu_pre_dispatch_comp);
            shader_module_ci.pCode = gpu_pre_dispatch_comp;
            result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &common_dispatch_resources.shader_module);
            if (result != VK_SUCCESS) {
                ReportSetupProblem(device, "Unable to create shader module. Aborting GPU-AV");
                aborted = true;
                return;
            }

            // Create pipeline
            VkPipelineShaderStageCreateInfo pipeline_stage_ci = vku::InitStructHelper();
            pipeline_stage_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            pipeline_stage_ci.module = common_dispatch_resources.shader_module;
            pipeline_stage_ci.pName = "main";

            VkComputePipelineCreateInfo pipeline_ci = vku::InitStructHelper();
            pipeline_ci.stage = pipeline_stage_ci;
            pipeline_ci.layout = common_dispatch_resources.pipeline_layout;

            result = DispatchCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr,
                                                    &common_dispatch_resources.pipeline);
            if (result != VK_SUCCESS) {
                ReportSetupProblem(device, "Failed to create compute pipeline for pre dispatch validation.");
            }
        }

        common_dispatch_resources.initialized = true;
    }

    result = desc_set_manager->GetDescriptorSet(&out_dispatch_resources.desc_pool, common_dispatch_resources.ds_layout,
                                                &out_dispatch_resources.desc_set);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate descriptor set. Aborting GPU-AV");
        aborted = true;
        return;
    }

    const uint32_t buffer_count = 2;
    VkDescriptorBufferInfo buffer_infos[buffer_count] = {};
    // Error output buffer
    buffer_infos[0].buffer = output_block.buffer;
    buffer_infos[0].offset = 0;
    buffer_infos[0].range = VK_WHOLE_SIZE;
    buffer_infos[1].buffer = indirect_state->buffer;
    buffer_infos[1].offset = 0;
    buffer_infos[1].range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet desc_writes[buffer_count] = {};
    for (uint32_t i = 0; i < buffer_count; i++) {
        desc_writes[i] = vku::InitStructHelper();
        desc_writes[i].dstBinding = i;
        desc_writes[i].descriptorCount = 1;
        desc_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[i].pBufferInfo = &buffer_infos[i];
        desc_writes[i].dstSet = out_dispatch_resources.desc_set;
    }
    DispatchUpdateDescriptorSets(device, buffer_count, desc_writes, 0, nullptr);
}

void gpuav::Validator::AllocatePreTraceRaysValidationResources(const DeviceMemoryBlock &output_block,
                                                               const CmdIndirectState *indirect_state,
                                                               PreTraceRaysResources &out_trace_rays_resources) {
    // Create Shader(s), only ray gen for now ?
    // In the context of validating the indirect command buffer, it seems that a dispatch could be enough

    // Allocate descriptor set. Can I assume VK_EXT_descriptor_indexing is supported?
    VkResult result = VK_SUCCESS;
    if (!common_trace_rays_resources.initialized) {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr}  // output buffer
        };

        VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();
        ds_layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
        ds_layout_ci.pBindings = bindings.data();
        result = DispatchCreateDescriptorSetLayout(device, &ds_layout_ci, nullptr, &common_trace_rays_resources.ds_layout);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to create descriptor set layout. Aborting GPU-AV");
            aborted = true;
            return;
        }

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = out_trace_rays_resources.push_constant_words * sizeof(uint32_t);
        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        pipeline_layout_ci.pushConstantRangeCount = 1;
        pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &common_trace_rays_resources.ds_layout;
        result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &common_trace_rays_resources.pipeline_layout);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to create pipeline layout. Aborting GPU-AV");
            aborted = true;
            return;
        }

        VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
        shader_module_ci.codeSize = sizeof(gpu_pre_trace_rays_rgen);
        shader_module_ci.pCode = gpu_pre_trace_rays_rgen;
        result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &common_trace_rays_resources.shader_module);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to create ray tracing shader module. Aborting GPU-AV");
            aborted = true;
            return;
        }

        // Create pipeline
        VkPipelineShaderStageCreateInfo pipeline_stage_ci = vku::InitStructHelper();
        pipeline_stage_ci.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        pipeline_stage_ci.module = common_trace_rays_resources.shader_module;
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
        rt_pipeline_create_info.layout = common_trace_rays_resources.pipeline_layout;
        result = DispatchCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rt_pipeline_create_info, nullptr,
                                                      &common_trace_rays_resources.pipeline);

        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create ray tracing pipeline for pre trace rays validation. Aborting GPU-AV");
            aborted = true;
            return;
        }

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&rt_pipeline_props);
        DispatchGetPhysicalDeviceProperties2(physical_device, &props2);

        // Get shader group handles to fill shader binding table (SBT)
        const uint32_t shader_group_size_aligned =
            Align(rt_pipeline_props.shaderGroupHandleSize, rt_pipeline_props.shaderGroupHandleAlignment);
        const uint32_t sbt_size = 1 * shader_group_size_aligned;
        std::vector<uint8_t> sbt_host_storage(sbt_size);
        result = DispatchGetRayTracingShaderGroupHandlesKHR(device, common_trace_rays_resources.pipeline, 0,
                                                            rt_pipeline_create_info.groupCount, sbt_size, sbt_host_storage.data());
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to call vkGetRayTracingShaderGroupHandlesKHR. Aborting GPU-AV");
            aborted = true;
            return;
        }

        // Allocate buffer to SBT, and fill it with sbt_host_storage
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.size = 4096;
        buffer_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        uint32_t mem_type_index = 0;
        vmaFindMemoryTypeIndexForBufferInfo(vmaAllocator, &buffer_info, &alloc_info, &mem_type_index);
        VmaPoolCreateInfo pool_create_info = {};
        pool_create_info.memoryTypeIndex = mem_type_index;
        pool_create_info.blockSize = 0;
        pool_create_info.maxBlockCount = 0;
        pool_create_info.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
        result = vmaCreatePool(vmaAllocator, &pool_create_info, &common_trace_rays_resources.sbt_pool);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to create VMA memory pool for SBT. Aborting GPU-AV");
            aborted = true;
            return;
        }

        alloc_info.pool = common_trace_rays_resources.sbt_pool;  // #ARNO_TODO create dedicated pool
        result = vmaCreateBuffer(vmaAllocator, &buffer_info, &alloc_info, &common_trace_rays_resources.sbt_buffer,
                                 &common_trace_rays_resources.sbt_allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to allocate device memory for shader binding table. Aborting GPU-AV.", true);
            aborted = true;
            return;
        }

        uint8_t *mapped_sbt = nullptr;
        result = vmaMapMemory(vmaAllocator, common_trace_rays_resources.sbt_allocation, reinterpret_cast<void **>(&mapped_sbt));

        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to map shader binding table when creating trace rays validation resources.");
            aborted = true;
            return;
        }

        std::memcpy(mapped_sbt, sbt_host_storage.data(), rt_pipeline_props.shaderGroupHandleSize);

        vmaUnmapMemory(vmaAllocator, common_trace_rays_resources.sbt_allocation);

        common_trace_rays_resources.shader_group_handle_size_aligned = shader_group_size_aligned;

        // Retrieve SBT address
        const VkDeviceAddress sbt_address = GetBufferDeviceAddress(common_trace_rays_resources.sbt_buffer);
        assert(sbt_address != 0);
        if (sbt_address == 0) {
            ReportSetupProblem(device, "Retrieved SBT buffer device address is null. Aborting GPU-AV.");
            aborted = true;
            return;
        }
        assert(sbt_address == Align(sbt_address, static_cast<VkDeviceAddress>(rt_pipeline_props.shaderGroupBaseAlignment)));
        common_trace_rays_resources.sbt_address = sbt_address;

        common_trace_rays_resources.initialized = true;
    }

    result = desc_set_manager->GetDescriptorSet(&out_trace_rays_resources.desc_pool, common_trace_rays_resources.ds_layout,
                                                &out_trace_rays_resources.desc_set);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate descriptor set. Aborting GPU-AV");
        aborted = true;
        return;
    }

    constexpr uint32_t buffer_count = 1;
    VkDescriptorBufferInfo buffer_infos[buffer_count] = {};
    // Error output buffer
    buffer_infos[0].buffer = output_block.buffer;
    buffer_infos[0].offset = 0;
    buffer_infos[0].range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet desc_writes[buffer_count] = {};
    for (uint32_t i = 0; i < buffer_count; i++) {
        desc_writes[i] = vku::InitStructHelper();
        desc_writes[i].dstBinding = i;
        desc_writes[i].descriptorCount = 1;
        desc_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[i].pBufferInfo = &buffer_infos[i];
        desc_writes[i].dstSet = out_trace_rays_resources.desc_set;
    }
    DispatchUpdateDescriptorSets(device, buffer_count, desc_writes, 0, nullptr);

    // Save parameters for error message
    out_trace_rays_resources.indirect_device_address = indirect_state->indirectDeviceAddress;
}

void gpuav::Validator::AllocateValidationResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point,
                                                   vvl::Func command, const CmdIndirectState *indirect_state) {
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        return;
    }
    VkResult result;

    if (aborted) return;

    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(device, "Unrecognized command buffer");
        aborted = true;
        return;
    }
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    auto const &last_bound = cb_node->lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;
    bool uses_robustness = false;
    const bool use_shader_objects = pipeline_state == nullptr;

    if (!pipeline_state && !last_bound.HasShaderObjects()) {
        ReportSetupProblem(device, "Neither pipeline state nor shader object states were found, aborting GPU-AV");
        aborted = true;
        return;
    }

    std::vector<VkDescriptorSet> desc_sets;
    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    result = desc_set_manager->GetDescriptorSets(1, &desc_pool, debug_desc_layout, &desc_sets);
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate descriptor sets. Device could become unstable.");
        aborted = true;
        return;
    }

    // Allocate memory for the output block that the gpu will use to return any error information
    DeviceMemoryBlock output_block = {};
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = output_buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.pool = output_buffer_pool;
    result = vmaCreateBuffer(vmaAllocator, &buffer_info, &alloc_info, &output_block.buffer, &output_block.allocation, nullptr);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate device memory. Device could become unstable.", true);
        aborted = true;
        return;
    }

    uint32_t *output_buffer_ptr;
    result = vmaMapMemory(vmaAllocator, output_block.allocation, reinterpret_cast<void **>(&output_buffer_ptr));
    if (result == VK_SUCCESS) {
        memset(output_buffer_ptr, 0, output_buffer_size);
        if (gpuav_settings.validate_descriptors) {
            uses_robustness = (enabled_features.robustBufferAccess || enabled_features.robustBufferAccess2 ||
                               (pipeline_state && pipeline_state->uses_pipeline_robustness));
            output_buffer_ptr[spvtools::kDebugOutputFlagsOffset] = spvtools::kInstBufferOOBEnable;
        }
        vmaUnmapMemory(vmaAllocator, output_block.allocation);
    } else {
        ReportSetupProblem(device, "Unable to map device memory allocated for output buffer. Device could become unstable.", true);
        aborted = true;
        return;
    }

    PreDrawResources draw_resources = {};
    PreDispatchResources dispatch_resources = {};
    PreTraceRaysResources trace_rays_resources = {};

    if (gpuav_settings.validate_indirect_buffer &&
        ((command == Func::vkCmdDrawIndirectCount || command == Func::vkCmdDrawIndirectCountKHR ||
          command == Func::vkCmdDrawIndexedIndirectCount || command == Func::vkCmdDrawIndexedIndirectCountKHR) ||
         ((command == Func::vkCmdDrawIndirect || command == Func::vkCmdDrawIndexedIndirect) &&
          !(enabled_features.drawIndirectFirstInstance)))) {
        // Insert a draw that can examine some device memory right before the draw we're validating (Pre Draw Validation)
        //
        // NOTE that this validation does not attempt to abort invalid api calls as most other validation does. A crash
        // or DEVICE_LOST resulting from the invalid call will prevent preceeding validation errors from being reported.

        assert(bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS);
        assert(indirect_state != NULL);
        VkPipeline validation_pipeline = VK_NULL_HANDLE;
        AllocatePreDrawValidationResources(output_block, cb_node->activeRenderPass.get()->renderPass(), use_shader_objects,
                                           &validation_pipeline, indirect_state, draw_resources);
        if (aborted) return;

        // Save current graphics pipeline state
        RestorablePipelineState restorable_state(cb_node.get(), VK_PIPELINE_BIND_POINT_GRAPHICS);

        // Save parameters for error message
        draw_resources.indirect_buffer = indirect_state->buffer;
        draw_resources.indirect_buffer_offset = indirect_state->offset;
        draw_resources.indirect_buffer_stride = indirect_state->stride;

        uint32_t push_constants[draw_resources.push_constant_words] = {};
        if (command == Func::vkCmdDrawIndirectCount || command == Func::vkCmdDrawIndirectCountKHR ||
            command == Func::vkCmdDrawIndexedIndirectCount || command == Func::vkCmdDrawIndexedIndirectCountKHR) {
            // Validate count buffer
            if (indirect_state->count_buffer_offset > std::numeric_limits<uint32_t>::max()) {
                ReportSetupProblem(device,
                                   "Count buffer offset is larger than can be contained in an unsigned int. Aborting GPU-AV");
                aborted = true;
                return;
            }

            // Buffer size must be >= (stride * (drawCount - 1) + offset + sizeof(VkDrawIndirectCommand))
            uint32_t struct_size;
            if (command == Func::vkCmdDrawIndirectCount || command == Func::vkCmdDrawIndirectCountKHR) {
                struct_size = sizeof(VkDrawIndirectCommand);
            } else {
                assert(command == Func::vkCmdDrawIndexedIndirectCount || command == Func::vkCmdDrawIndexedIndirectCountKHR);
                struct_size = sizeof(VkDrawIndexedIndirectCommand);
            }
            auto buffer_state = Get<BUFFER_STATE>(indirect_state->buffer);
            uint32_t max_count;
            uint64_t bufsize = buffer_state->createInfo.size;
            uint64_t first_command_bytes = struct_size + indirect_state->offset;
            if (first_command_bytes > bufsize) {
                max_count = 0;
            } else {
                max_count = 1 + static_cast<uint32_t>(std::floor(((bufsize - first_command_bytes) / indirect_state->stride)));
            }
            draw_resources.indirect_buffer_size = buffer_state->createInfo.size;

            assert(phys_dev_props.limits.maxDrawIndirectCount > 0);
            push_constants[0] = phys_dev_props.limits.maxDrawIndirectCount;
            push_constants[1] = max_count;
            push_constants[2] = static_cast<uint32_t>((indirect_state->count_buffer_offset / sizeof(uint32_t)));
        } else {
            // Validate buffer for firstInstance check instead of count buffer check
            push_constants[0] = 0;
            push_constants[1] = indirect_state->draw_count;
            if (command == Func::vkCmdDrawIndirect) {
                push_constants[2] = static_cast<uint32_t>(
                    ((indirect_state->offset + offsetof(struct VkDrawIndirectCommand, firstInstance)) / sizeof(uint32_t)));
            } else {
                assert(command == Func::vkCmdDrawIndexedIndirect);
                push_constants[2] = static_cast<uint32_t>(
                    ((indirect_state->offset + offsetof(struct VkDrawIndexedIndirectCommand, firstInstance)) / sizeof(uint32_t)));
            }
            push_constants[3] = (indirect_state->stride / sizeof(uint32_t));
        }

        // Insert diagnostic draw
        if (use_shader_objects) {
            VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
            DispatchCmdBindShadersEXT(cmd_buffer, 1u, &stage, &common_draw_resources.shader_object);
        } else {
            DispatchCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, validation_pipeline);
        }
        DispatchCmdPushConstants(cmd_buffer, common_draw_resources.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                 sizeof(push_constants), push_constants);
        DispatchCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, common_draw_resources.pipeline_layout, 0, 1,
                                      &draw_resources.desc_set, 0, nullptr);
        DispatchCmdDraw(cmd_buffer, 3, 1, 0, 0);

        // Restore the previous graphics pipeline state.
        restorable_state.Restore(cmd_buffer);
    } else if (gpuav_settings.validate_indirect_buffer && command == Func::vkCmdDispatchIndirect) {
        // Insert a dispatch that can examine some device memory right before the dispatch we're validating
        //
        // NOTE that this validation does not attempt to abort invalid api calls as most other validation does. A crash
        // or DEVICE_LOST resulting from the invalid call will prevent preceding validation errors from being reported.

        AllocatePreDispatchValidationResources(output_block, indirect_state, use_shader_objects, dispatch_resources);
        if (aborted) return;

        // Save current graphics pipeline state
        RestorablePipelineState restorable_state(cb_node.get(), VK_PIPELINE_BIND_POINT_COMPUTE);

        // Save parameters for error message
        dispatch_resources.indirect_buffer = indirect_state->buffer;
        dispatch_resources.indirect_buffer_offset = indirect_state->offset;

        uint32_t push_constants[dispatch_resources.push_constant_words] = {};
        push_constants[0] = phys_dev_props.limits.maxComputeWorkGroupCount[0];
        push_constants[1] = phys_dev_props.limits.maxComputeWorkGroupCount[1];
        push_constants[2] = phys_dev_props.limits.maxComputeWorkGroupCount[2];
        push_constants[3] = static_cast<uint32_t>((indirect_state->offset / sizeof(uint32_t)));

        // Insert diagnostic dispatch
        if (use_shader_objects) {
            VkShaderStageFlagBits stage = VK_SHADER_STAGE_COMPUTE_BIT;
            DispatchCmdBindShadersEXT(cmd_buffer, 1u, &stage, &common_dispatch_resources.shader_object);
        } else {
            DispatchCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, common_dispatch_resources.pipeline);
        }
        DispatchCmdPushConstants(cmd_buffer, common_dispatch_resources.pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                 sizeof(push_constants), push_constants);
        DispatchCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, common_dispatch_resources.pipeline_layout, 0, 1,
                                      &dispatch_resources.desc_set, 0, nullptr);
        DispatchCmdDispatch(cmd_buffer, 1, 1, 1);

        // Restore the previous compute pipeline state.
        restorable_state.Restore(cmd_buffer);
    } else if (gpuav_settings.validate_indirect_buffer &&
               (command == Func::vkCmdTraceRaysIndirectKHR /*|| command == Func::vkCmdTraceRaysIndirect2KHR */)) {
        AllocatePreTraceRaysValidationResources(output_block, indirect_state, trace_rays_resources);
        if (aborted) return;

        // Save current ray tracing pipeline state
        RestorablePipelineState restorable_state(cb_node.get(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

        // Push info needed for validation:
        // - the device address indirect data is read from
        // - the limits to check against
        uint32_t push_constants[trace_rays_resources.push_constant_words] = {};
        const uint64_t ray_query_dimension_max_width = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[0]) *
                                                       static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[0]);
        const uint64_t ray_query_dimension_max_height = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[1]) *
                                                        static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[1]);
        const uint64_t ray_query_dimension_max_depth = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[2]) *
                                                       static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[2]);
        // Need to put the buffer reference first otherwise it is incorrect, probably an alignment issue
        push_constants[0] = static_cast<uint32_t>(trace_rays_resources.indirect_device_address) & vvl::kU32Max;
        push_constants[1] = static_cast<uint32_t>(trace_rays_resources.indirect_device_address >> 32) & vvl::kU32Max;
        push_constants[2] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_width, vvl::kU32Max));
        push_constants[3] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_height, vvl::kU32Max));
        push_constants[4] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_depth, vvl::kU32Max));

        DispatchCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, common_trace_rays_resources.pipeline);
        DispatchCmdPushConstants(cmd_buffer, common_trace_rays_resources.pipeline_layout, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0,
                                 sizeof(push_constants), push_constants);
        DispatchCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                                      common_trace_rays_resources.pipeline_layout, 0, 1, &trace_rays_resources.desc_set, 0,
                                      nullptr);
        VkStridedDeviceAddressRegionKHR ray_gen_sbt{};
        assert(common_trace_rays_resources.sbt_address != 0);
        ray_gen_sbt.deviceAddress = common_trace_rays_resources.sbt_address;
        ray_gen_sbt.stride = common_trace_rays_resources.shader_group_handle_size_aligned;
        ray_gen_sbt.size = common_trace_rays_resources.shader_group_handle_size_aligned;

        VkStridedDeviceAddressRegionKHR empty_sbt{};
        DispatchCmdTraceRaysKHR(cmd_buffer, &ray_gen_sbt, &empty_sbt, &empty_sbt, &empty_sbt, 1, 1, 1);

        // Restore the previous ray tracing pipeline state.
        restorable_state.Restore(cmd_buffer);
    }

    // Write the descriptor that will be used to check for OOB accesses
    {
        VkDescriptorBufferInfo output_desc_buffer_info = {};
        output_desc_buffer_info.range = output_buffer_size;
        output_desc_buffer_info.buffer = output_block.buffer;
        output_desc_buffer_info.offset = 0;

        std::array<VkWriteDescriptorSet, 3> desc_writes = {};
        VkDescriptorBufferInfo di_input_desc_buffer_info = {};
        VkDescriptorBufferInfo bda_input_desc_buffer_info = {};

        desc_writes[0] = vku::InitStructHelper();
        desc_writes[0].descriptorCount = 1;
        desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[0].pBufferInfo = &output_desc_buffer_info;
        desc_writes[0].dstSet = desc_sets[0];

        uint32_t desc_count = 1;

        if (cb_node->current_bindless_buffer != VK_NULL_HANDLE) {
            di_input_desc_buffer_info.range = VK_WHOLE_SIZE;
            di_input_desc_buffer_info.buffer = cb_node->current_bindless_buffer;
            di_input_desc_buffer_info.offset = 0;

            desc_writes[desc_count] = vku::InitStructHelper();
            desc_writes[desc_count].dstBinding = 1;
            desc_writes[desc_count].descriptorCount = 1;
            desc_writes[desc_count].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            desc_writes[desc_count].pBufferInfo = &di_input_desc_buffer_info;
            desc_writes[desc_count].dstSet = desc_sets[0];
            desc_count++;
        }

        if (buffer_device_address_enabled) {
            bda_input_desc_buffer_info.range = app_bda_buffer_size;
            bda_input_desc_buffer_info.buffer = app_buffer_device_addresses.buffer;
            bda_input_desc_buffer_info.offset = 0;

            desc_writes[desc_count] = vku::InitStructHelper();
            desc_writes[desc_count].dstBinding = 2;
            desc_writes[desc_count].descriptorCount = 1;
            desc_writes[desc_count].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            desc_writes[desc_count].pBufferInfo = &bda_input_desc_buffer_info;
            desc_writes[desc_count].dstSet = desc_sets[0];
            desc_count++;
        }

        DispatchUpdateDescriptorSets(device, desc_count, desc_writes.data(), 0, NULL);
    }

    const auto pipeline_layout =
        pipeline_state ? pipeline_state->PipelineLayoutState() : Get<PIPELINE_LAYOUT_STATE>(last_bound.pipeline_layout);
    // If GPL is used, it's possible the pipeline layout used at pipeline creation time is null. If CmdBindDescriptorSets has
    // not been called yet (i.e., state.pipeline_null), then fall back to the layout associated with pre-raster state.
    // PipelineLayoutState should be used for the purposes of determining the number of sets in the layout, but this layout
    // may be a "pseudo layout" used to represent the union of pre-raster and fragment shader layouts, and therefore have a
    // null handle.
    VkPipelineLayout pipeline_layout_handle = VK_NULL_HANDLE;
    if (last_bound.pipeline_layout) {
        pipeline_layout_handle = last_bound.pipeline_layout;
    } else if (pipeline_state && !pipeline_state->PreRasterPipelineLayoutState()->Destroyed()) {
        pipeline_layout_handle = pipeline_state->PreRasterPipelineLayoutState()->layout();
    }
    if ((pipeline_layout && pipeline_layout->set_layouts.size() <= desc_set_bind_index) &&
        pipeline_layout_handle != VK_NULL_HANDLE) {
        DispatchCmdBindDescriptorSets(cmd_buffer, bind_point, pipeline_layout_handle, desc_set_bind_index, 1, desc_sets.data(), 0,
                                      nullptr);
    } else {
        // If no pipeline layout was bound when using shader objects that don't use any descriptor set, bind the debug pipeline
        // layout
        DispatchCmdBindDescriptorSets(cmd_buffer, bind_point, debug_pipeline_layout, desc_set_bind_index, 1, desc_sets.data(), 0,
                                      nullptr);
    }

    if (pipeline_state && pipeline_layout_handle == VK_NULL_HANDLE) {
        ReportSetupProblem(device, "Unable to find pipeline layout to bind debug descriptor set. Aborting GPU-AV");
        aborted = true;
        vmaDestroyBuffer(vmaAllocator, output_block.buffer, output_block.allocation);
    } else {
        // It is possible to have no descriptor sets bound, for example if using push constants.
        uint32_t di_buf_index =
            cb_node->di_input_buffer_list.size() > 0 ? uint32_t(cb_node->di_input_buffer_list.size()) - 1 : vvl::kU32Max;
        // Record buffer and memory info in CB state tracking
        cb_node->per_draw_command_infos.emplace_back(output_block, draw_resources, dispatch_resources, trace_rays_resources,
                                                     desc_sets[0], desc_pool, bind_point, uses_robustness, command, di_buf_index);
    }
}

// Generate the part of the message describing the violation.
bool gpuav::Validator::GenerateValidationMessage(const uint32_t *debug_record, const CommandInfo &cmd_info,
                                                 const std::vector<DescSetState> &descriptor_sets, std::string &out_error_msg,
                                                 std::string &out_vuid_msg, bool &out_oob_access) const {
    using namespace spvtools;
    using namespace glsl;
    std::ostringstream strm;
    bool return_code = true;
    const GpuVuid vuid = GetGpuVuid(cmd_info.command);
    out_oob_access = false;
    switch (debug_record[kInstValidationOutError]) {
        case kInstErrorBindlessBounds: {
            strm << "(set = " << debug_record[kInstBindlessBoundsOutDescSet]
                 << ", binding = " << debug_record[kInstBindlessBoundsOutDescBinding] << ") Index of "
                 << debug_record[kInstBindlessBoundsOutDescIndex] << " used to index descriptor array of length "
                 << debug_record[kInstBindlessBoundsOutDescBound] << ".";
            out_vuid_msg = "UNASSIGNED-Descriptor index out of bounds";
        } break;
        case kInstErrorBindlessUninit: {
            strm << "(set = " << debug_record[kInstBindlessUninitOutDescSet]
                 << ", binding = " << debug_record[kInstBindlessUninitOutBinding] << ") Descriptor index "
                 << debug_record[kInstBindlessUninitOutDescIndex] << " is uninitialized.";
            out_vuid_msg = "UNASSIGNED-Descriptor uninitialized";
        } break;
        case kInstErrorBindlessDestroyed: {
            strm << "(set = " << debug_record[kInstBindlessUninitOutDescSet]
                 << ", binding = " << debug_record[kInstBindlessUninitOutBinding] << ") Descriptor index "
                 << debug_record[kInstBindlessUninitOutDescIndex] << " references a resource that was destroyed.";
            out_vuid_msg = "UNASSIGNED-Descriptor destroyed";
        } break;
        case kInstErrorBuffAddrUnallocRef: {
            out_oob_access = true;
            uint64_t *ptr = (uint64_t *)&debug_record[kInstBuffAddrUnallocOutDescPtrLo];
            strm << "Device address 0x" << std::hex << *ptr << " access out of bounds. ";
            out_vuid_msg = "UNASSIGNED-Device address out of bounds";
        } break;
        case kInstErrorOOB: {
            const uint32_t set_num = debug_record[kInstBindlessBuffOOBOutDescSet];
            const uint32_t binding_num = debug_record[kInstBindlessBuffOOBOutDescBinding];
            const uint32_t desc_index = debug_record[kInstBindlessBuffOOBOutDescIndex];
            const uint32_t size = debug_record[kInstBindlessBuffOOBOutBuffSize];
            const uint32_t offset = debug_record[kInstBindlessBuffOOBOutBuffOff];
            const auto *binding_state = descriptor_sets[set_num].state->GetBinding(binding_num);
            assert(binding_state);
            if (size == 0) {
                strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                     << " is uninitialized.";
                out_vuid_msg = "UNASSIGNED-Descriptor uninitialized";
                break;
            }
            out_oob_access = true;
            auto desc_class = binding_state->descriptor_class;
            if (desc_class == vvl::DescriptorClass::Mutable) {
                desc_class = static_cast<const vvl::MutableBinding *>(binding_state)->descriptors[desc_index].ActiveClass();
            }

            switch (desc_class) {
                case vvl::DescriptorClass::GeneralBuffer:
                    strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                         << " access out of bounds. Descriptor size is " << size << " and highest byte accessed was " << offset;
                    if (binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                        binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                        out_vuid_msg = vuid.uniform_access_oob;
                    } else {
                        out_vuid_msg = vuid.storage_access_oob;
                    }
                    break;
                case vvl::DescriptorClass::TexelBuffer:
                    strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                         << " access out of bounds. Descriptor size is " << size << " texels and highest texel accessed was "
                         << offset;
                    if (binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
                        out_vuid_msg = vuid.uniform_access_oob;
                    } else {
                        out_vuid_msg = vuid.storage_access_oob;
                    }
                    break;
                default:
                    // other OOB checks are not implemented yet
                    assert(false);
            }
        } break;
        case kInstErrorPreDrawValidate: {
            // Buffer size must be >= (stride * (drawCount - 1) + offset + sizeof(VkDrawIndexedIndirectCommand))
            if (debug_record[kPreValidateSubError] == pre_draw_count_exceeds_bufsize_error) {
                uint32_t count = debug_record[kPreValidateSubError + 1];
                uint32_t stride = cmd_info.draw_resources.indirect_buffer_stride;
                uint32_t offset = static_cast<uint32_t>(cmd_info.draw_resources.indirect_buffer_offset);
                uint32_t draw_size = (stride * (count - 1) + offset + sizeof(VkDrawIndexedIndirectCommand));
                strm << "Indirect draw count of " << count << " would exceed buffer size "
                     << cmd_info.draw_resources.indirect_buffer_size << " of buffer " << cmd_info.draw_resources.indirect_buffer
                     << " stride = " << stride << " offset = " << offset
                     << " (stride * (drawCount - 1) + offset + sizeof(VkDrawIndexedIndirectCommand)) = " << draw_size;
                if (count == 1) {
                    out_vuid_msg = vuid.count_exceeds_bufsize_1;
                } else {
                    out_vuid_msg = vuid.count_exceeds_bufsize;
                }
            } else if (debug_record[kPreValidateSubError] == pre_draw_count_exceeds_limit_error) {
                uint32_t count = debug_record[kPreValidateSubError + 1];
                strm << "Indirect draw count of " << count << " would exceed maxDrawIndirectCount limit of "
                     << phys_dev_props.limits.maxDrawIndirectCount;
                out_vuid_msg = vuid.count_exceeds_device_limit;
            } else if (debug_record[kPreValidateSubError] == pre_draw_first_instance_error) {
                uint32_t index = debug_record[kPreValidateSubError + 1];
                strm << "The drawIndirectFirstInstance feature is not enabled, but the firstInstance member of the "
                     << ((cmd_info.command == vvl::Func::vkCmdDrawIndirect) ? "VkDrawIndirectCommand"
                                                                            : "VkDrawIndexedIndirectCommand")
                     << " structure at index " << index << " is not zero";
                out_vuid_msg = vuid.first_instance_not_zero;
            }
            return_code = false;
        } break;
        case kInstErrorPreDispatchValidate: {
            if (debug_record[kPreValidateSubError] == pre_dispatch_count_exceeds_limit_x_error) {
                uint32_t count = debug_record[kPreValidateSubError + 1];
                strm << "Indirect dispatch VkDispatchIndirectCommand::x of " << count
                     << " would exceed maxComputeWorkGroupCount[0] limit of " << phys_dev_props.limits.maxComputeWorkGroupCount[0];
                out_vuid_msg = vuid.group_exceeds_device_limit_x;
            } else if (debug_record[kPreValidateSubError] == pre_dispatch_count_exceeds_limit_y_error) {
                uint32_t count = debug_record[kPreValidateSubError + 1];
                strm << "Indirect dispatch VkDispatchIndirectCommand:y of " << count
                     << " would exceed maxComputeWorkGroupCount[1] limit of " << phys_dev_props.limits.maxComputeWorkGroupCount[1];
                out_vuid_msg = vuid.group_exceeds_device_limit_y;
            } else if (debug_record[kPreValidateSubError] == pre_dispatch_count_exceeds_limit_z_error) {
                uint32_t count = debug_record[kPreValidateSubError + 1];
                strm << "Indirect dispatch VkDispatchIndirectCommand::z of " << count
                     << " would exceed maxComputeWorkGroupCount[2] limit of " << phys_dev_props.limits.maxComputeWorkGroupCount[2];
                out_vuid_msg = vuid.group_exceeds_device_limit_z;
            }
            return_code = false;
        } break;

        case kInstErrorPreTraceRaysKhrValidate: {
            if (debug_record[kPreValidateSubError] == pre_trace_rays_query_dimensions_exceeds_width_limit) {
                uint32_t width = debug_record[kPreValidateSubError + 1];
                strm << "Indirect trace rays of VkTraceRaysIndirectCommandKHR::width of " << width
                     << " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[0] * "
                        "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[0] limit of "
                     << static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[0]) *
                            static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[0]);
                out_vuid_msg = vuid.trace_rays_width_exceeds_device_limit;
            } else if (debug_record[kPreValidateSubError] == pre_trace_rays_query_dimensions_exceeds_height_limit) {
                uint32_t height = debug_record[kPreValidateSubError + 1];
                strm << "Indirect trace rays of VkTraceRaysIndirectCommandKHR::height of " << height
                     << " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[1] * "
                        "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[1] limit of "
                     << static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[1]) *
                            static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[1]);
                out_vuid_msg = vuid.trace_rays_height_exceeds_device_limit;
            } else if (debug_record[kPreValidateSubError] == pre_trace_rays_query_dimensions_exceeds_depth_limit) {
                uint32_t depth = debug_record[kPreValidateSubError + 1];
                strm << "Indirect trace rays of VkTraceRaysIndirectCommandKHR::depth of " << depth
                     << " would exceed VkPhysicalDeviceLimits::maxComputeWorkGroupCount[2] * "
                        "VkPhysicalDeviceLimits::maxComputeWorkGroupSize[2] limit of "
                     << static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[2]) *
                            static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[2]);
                out_vuid_msg = vuid.trace_rays_depth_exceeds_device_limit;
            }
            return_code = false;
        } break;
        default: {
            strm << "Internal Error (unexpected error type = " << debug_record[kInstValidationOutError] << "). ";
            out_vuid_msg = "UNASSIGNED-Internal Error";
            assert(false);
        } break;
    }
    out_error_msg = strm.str();
    return return_code;
}

// Pull together all the information from the debug record to build the error message strings,
// and then assemble them into a single message string.
// Retrieve the shader program referenced by the unique shader ID provided in the debug record.
// We had to keep a copy of the shader program with the same lifecycle as the pipeline to make
// sure it is available when the pipeline is submitted.  (The ShaderModule tracking object also
// keeps a copy, but it can be destroyed after the pipeline is created and before it is submitted.)
//
void gpuav::Validator::AnalyzeAndGenerateMessages(CommandBuffer &command_buffer, VkQueue queue, CommandInfo &cmd_info,
                                                  uint32_t operation_index, uint32_t *const debug_output_buffer,
                                                  const std::vector<DescSetState> &descriptor_sets, const Location &loc) {
    const uint32_t total_words = debug_output_buffer[spvtools::kDebugOutputSizeOffset];
    bool oob_access;
    // A zero here means that the shader instrumentation didn't write anything.
    // If you have nothing to say, don't say it here.
    if (0 == total_words) {
        return;
    }
    // The second word in the debug output buffer is the number of words that would have
    // been written by the shader instrumentation, if there was enough room in the buffer we provided.
    // The number of words actually written by the shaders is determined by the size of the buffer
    // we provide via the descriptor.  So, we process only the number of words that can fit in the
    // buffer.
    // Each "report" written by the shader instrumentation is considered a "record".  This function
    // is hard-coded to process only one record because it expects the buffer to be large enough to
    // hold only one record.  If there is a desire to process more than one record, this function needs
    // to be modified to loop over records and the buffer size increased.
    std::string error_msg;
    std::string stage_message;
    std::string common_message;
    std::string filename_message;
    std::string source_message;
    std::string vuid_msg;
    VkShaderModule shader_module_handle = VK_NULL_HANDLE;
    VkPipeline pipeline_handle = VK_NULL_HANDLE;
    VkShaderEXT shader_object_handle = VK_NULL_HANDLE;
    vvl::span<const uint32_t> pgm;
    // The first record starts at this offset after the total_words.
    const uint32_t *debug_record = &debug_output_buffer[spvtools::kDebugOutputDataOffset];
    // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
    // by the instrumented shader.
    auto it = shader_map.find(debug_record[glsl::kInstCommonOutShaderId]);
    if (it != shader_map.end()) {
        shader_module_handle = it->second.shader_module;
        pipeline_handle = it->second.pipeline;
        shader_object_handle = it->second.shader_object;
        pgm = it->second.pgm;
    }
    const bool gen_full_message =
        GenerateValidationMessage(debug_record, cmd_info, descriptor_sets, error_msg, vuid_msg, oob_access);
    if (gen_full_message) {
        UtilGenerateStageMessage(debug_record, stage_message);
        UtilGenerateCommonMessage(report_data, command_buffer.commandBuffer(), debug_record, shader_module_handle, pipeline_handle,
                                  shader_object_handle, cmd_info.pipeline_bind_point, operation_index, common_message);
        UtilGenerateSourceMessages(pgm, debug_record, false, filename_message, source_message);
        if (cmd_info.uses_robustness && oob_access) {
            if (gpuav_settings.warn_on_robust_oob) {
                LogWarning(vuid_msg.c_str(), queue, loc, "%s %s %s %s%s", error_msg.c_str(), common_message.c_str(),
                           stage_message.c_str(), filename_message.c_str(), source_message.c_str());
            }
        } else {
            LogError(vuid_msg.c_str(), queue, loc, "%s %s %s %s%s", error_msg.c_str(), common_message.c_str(),
                     stage_message.c_str(), filename_message.c_str(), source_message.c_str());
        }
    } else {
        LogError(vuid_msg.c_str(), queue, loc, "%s", error_msg.c_str());
    }

    // Clear the written size and any error messages. Note that this preserves the first word, which contains flags.
    const uint32_t words_to_clear = std::min(total_words, output_buffer_size - spvtools::kDebugOutputDataOffset);
    debug_output_buffer[spvtools::kDebugOutputSizeOffset] = 0;
    memset(&debug_output_buffer[spvtools::kDebugOutputDataOffset], 0, sizeof(uint32_t) * words_to_clear);
}
