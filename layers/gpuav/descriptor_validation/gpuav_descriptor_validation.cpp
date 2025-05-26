/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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

#include "gpuav/descriptor_validation/gpuav_descriptor_validation.h"

#include "containers/custom_containers.h"
#include "drawdispatch/descriptor_validator.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/shader_module.h"

#include "profiling/profiling.h"
#include "state_tracker/shader_object_state.h"

namespace gpuav {
namespace descriptor {

void UpdateBoundDescriptorsDescriptorChecks(Validator &gpuav, CommandBufferSubState &cb_state, const LastBound &last_bound,
                                            DescriptorSetBindingCommand &descriptor_binding_cmd, const Location &loc) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        return;
    }

    // Create a new buffer to hold our BDA pointers
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = sizeof(glsl::DescriptorStateSSBO);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.pool = VK_NULL_HANDLE;
    const bool success = descriptor_binding_cmd.descritpor_state_ssbo_buffer.Create(&buffer_info, &alloc_info);
    if (!success) {
        return;
    }

    auto ssbo_buffer_ptr = (glsl::DescriptorStateSSBO *)descriptor_binding_cmd.descritpor_state_ssbo_buffer.GetMappedPtr();
    memset(ssbo_buffer_ptr, 0, sizeof(glsl::DescriptorStateSSBO));

    cb_state.descriptor_indexing_buffer = descriptor_binding_cmd.descritpor_state_ssbo_buffer.VkHandle();

    ssbo_buffer_ptr->initialized_status = gpuav.desc_heap_->GetDeviceAddress();

    const size_t number_of_sets = last_bound.ds_slots.size();
    for (uint32_t i = 0; i < number_of_sets; i++) {
        const auto &ds_slot = last_bound.ds_slots[i];
        if (!ds_slot.ds_state) {
            continue;  // can have gaps in descriptor sets
        }

        // If update after bind, wait until we process things in UpdateDescriptorStateSSBO()
        if (!ds_slot.ds_state->IsUpdateAfterBind()) {
            ssbo_buffer_ptr->descriptor_set_types[i] = SubState(*ds_slot.ds_state).GetTypeAddress(gpuav, loc);
        }
    }
}

void UpdateBoundDescriptors(Validator &gpuav, CommandBufferSubState &cb_state, VkPipelineBindPoint pipeline_bind_point,
                            const Location &loc) {
    if (!gpuav.gpuav_settings.shader_instrumentation.post_process_descriptor_indexing &&
        !gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        return;
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    auto const &last_bound = cb_state.base.lastBound[lv_bind_point];

    const size_t number_of_sets = last_bound.ds_slots.size();
    if (number_of_sets == 0) {
        return;  // empty bind
    } else if (number_of_sets > glsl::kDebugInputBindlessMaxDescSets) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Binding more than kDebugInputBindlessMaxDescSets limit");
        return;
    }

    DescriptorSetBindingCommand descriptor_binding_cmd(gpuav);
    descriptor_binding_cmd.bound_descriptor_sets.reserve(number_of_sets);
    // Currently we loop through the sets multiple times to reduce complexity and seperate the various parts, can revisit if we find
    // this is actually a perf bottleneck (assume number of sets are low as people we will then to have a single large set)
    for (uint32_t i = 0; i < number_of_sets; i++) {
        const auto &ds_slot = last_bound.ds_slots[i];
        descriptor_binding_cmd.bound_descriptor_sets.emplace_back(ds_slot.ds_state);
    }

    UpdateBoundDescriptorsDescriptorChecks(gpuav, cb_state, last_bound, descriptor_binding_cmd, loc);

    // #ARNO_TODO should not be a GetOrCreate but a TryGet: only care about creating this cache if it is used.
    // => need to update `UpdateBoundDescriptorsDescriptorChecks` and `UpdateDescriptorStateSSBO` same as I did for post processing
    DescriptorSetBindings &desc_set_bindings = cb_state.shared_resources_cache.GetOrCreate<DescriptorSetBindings>();
    for (auto &descriptor_binding_func : desc_set_bindings.on_update_bound_descriptor_sets) {
        descriptor_binding_func(cb_state, descriptor_binding_cmd);
    }
    desc_set_bindings.descriptor_set_binding_commands.emplace_back(std::move(descriptor_binding_cmd));
}

// For the given command buffer, map its debug data buffers and update the status of any update after bind descriptors
[[nodiscard]] bool UpdateDescriptorStateSSBO(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc) {
    const bool need_descriptor_checks_update = gpuav.gpuav_settings.shader_instrumentation.descriptor_checks;
    if (!need_descriptor_checks_update) {
        return true;
    }

    DescriptorSetBindings *desc_set_bindings = cb_state.shared_resources_cache.TryGet<DescriptorSetBindings>();
    if (!desc_set_bindings) {
        return true;
    }
    for (DescriptorSetBindingCommand &descriptor_command_binding : desc_set_bindings->descriptor_set_binding_commands) {
        glsl::DescriptorStateSSBO *desc_state_ssbo_ptr =
            (glsl::DescriptorStateSSBO *)descriptor_command_binding.descritpor_state_ssbo_buffer.GetMappedPtr();
        for (size_t i = 0; i < descriptor_command_binding.bound_descriptor_sets.size(); i++) {
            // Perfectly can have gaps in descriptor sets bindings
            if (!descriptor_command_binding.bound_descriptor_sets[i]) {
                continue;
            }
            DescriptorSetSubState &desc_set_state = SubState(*descriptor_command_binding.bound_descriptor_sets[i]);
            if (need_descriptor_checks_update) {
                desc_state_ssbo_ptr->descriptor_set_types[i] = desc_set_state.GetTypeAddress(gpuav, loc);
            }
        }
    }
    return true;
}
}  // namespace descriptor
}  // namespace gpuav
