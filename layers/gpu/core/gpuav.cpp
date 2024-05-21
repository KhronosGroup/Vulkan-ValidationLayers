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
#include "chassis/chassis_modification_state.h"
#include "containers/custom_containers.h"
#include "generated/layer_chassis_dispatch.h"
#include "gpu/core/gpuav_constants.h"
#include "gpu/core/gpuav.h"
#include "gpu/resources/gpuav_subclasses.h"

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

bool Validator::AllocateOutputMem(gpu::DeviceMemoryBlock &output_mem, const Location &loc) {
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
