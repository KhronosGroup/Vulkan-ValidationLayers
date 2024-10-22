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

#include "gpu/descriptor_validation/gpuav_descriptor_validation.h"

#include "drawdispatch/descriptor_validator.h"
#include "gpu/core/gpuav.h"
#include "gpu/resources/gpuav_subclasses.h"
#include "gpu/resources/gpuav_shader_resources.h"

namespace gpuav {
namespace descriptor {
void UpdateBoundPipeline(Validator &gpuav, CommandBuffer &cb_state, VkPipelineBindPoint pipeline_bind_point, VkPipeline pipeline,
                         const Location &loc) {
    // Currently this is only for updating the binding_req_map which is used for post processing only
    if (!gpuav.gpuav_settings.shader_instrumentation.post_process_descriptor_index) return;

    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];
    // Should have just been updated
    if (!last_bound.pipeline_state) {
        gpuav.InternalError(pipeline, loc, "Unrecognized pipeline");
        return;
    }

    // Catch if pipeline is bound before any descriptor sets
    if (cb_state.descriptor_command_bindings.empty()) {
        return;
    }

    // Update the last vkCmdBindDescriptorSet with the new pipeline
    auto &bound_descriptor_sets = cb_state.descriptor_command_bindings.back().bound_descriptor_sets;

    // If the user calls vkCmdBindDescriptorSet::firstSet to a non-zero value, these indexes don't line up
    size_t update_index = 0;
    for (uint32_t i = 0; i < last_bound.per_set.size(); i++) {
        if (last_bound.per_set[i].bound_descriptor_set) {
            auto slot = last_bound.pipeline_state->active_slots.find(i);
            if (slot != last_bound.pipeline_state->active_slots.end()) {
                if (update_index >= bound_descriptor_sets.size()) {
                    // TODO - Hit crash running with Dota2, this shouldn't happen, need to look into
                    continue;
                }
                bound_descriptor_sets[update_index++].binding_req_map = slot->second;
            }
        }
    }
}

void UpdateBoundDescriptors(Validator &gpuav, CommandBuffer &cb_state, VkPipelineBindPoint pipeline_bind_point,
                            const Location &loc) {
    const bool need_post_processing = gpuav.gpuav_settings.shader_instrumentation.post_process_descriptor_index;
    const bool need_descriptor_checks = gpuav.gpuav_settings.shader_instrumentation.bindless_descriptor;
    if (!need_descriptor_checks && !need_post_processing) return;

    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];

    const size_t number_of_sets = last_bound.per_set.size();
    if (number_of_sets == 0) {
        return;  // empty bind
    } else if (number_of_sets > glsl::kDebugInputBindlessMaxDescSets) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Binding more than kDebugInputBindlessMaxDescSets limit");
        return;
    }

    // Figure out how much memory we need for the input block based on how many sets and bindings there are
    // and how big each of the bindings is
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = sizeof(glsl::DescriptorStateSSBO);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.pool = VK_NULL_HANDLE;
    DescriptorCommandBinding descriptor_command_binding(gpuav);

    // Allocate buffer for device addresses of the input buffer for each descriptor set.  This is the buffer written to each
    // draw's descriptor set.
    descriptor_command_binding.ssbo_block.CreateBuffer(loc, &buffer_info, &alloc_info);

    auto ssbo_memory = (glsl::DescriptorStateSSBO *)descriptor_command_binding.ssbo_block.MapMemory(loc);

    memset(ssbo_memory, 0, static_cast<size_t>(buffer_info.size));
    cb_state.current_bindless_buffer = descriptor_command_binding.ssbo_block.Buffer();

    ssbo_memory->initialized_status = gpuav.desc_heap_->GetDeviceAddress();
    descriptor_command_binding.bound_descriptor_sets.reserve(descriptor_command_binding.bound_descriptor_sets.size() +
                                                             last_bound.per_set.size());
    for (uint32_t i = 0; i < last_bound.per_set.size(); i++) {
        const auto &last_bound_set = last_bound.per_set[i];
        if (!last_bound_set.bound_descriptor_set) {
            continue;
        }

        DescriptorCommandBountSet bound_descriptor_set;
        bound_descriptor_set.state = std::static_pointer_cast<DescriptorSet>(last_bound_set.bound_descriptor_set);
        DescriptorSet &ds_state = *bound_descriptor_set.state;
        // The pipeline might not have been bound yet, so will need to update binding_req_map later
        if (last_bound.pipeline_state) {
            auto slot = last_bound.pipeline_state->active_slots.find(i);
            if (slot != last_bound.pipeline_state->active_slots.end()) {
                bound_descriptor_set.binding_req_map = slot->second;
            }
        }

        ssbo_memory->desc_sets[i].descriptor_index_lut = ds_state.GetIndexLUTAddress(gpuav, loc);
        if (need_post_processing) {
            ssbo_memory->desc_sets[i].descriptor_index_post_process = ds_state.GetPostProcessBuffer(gpuav, loc);
        }

        // If update after bind, wait until we process things in UpdateDescriptorStateSSBO()
        if (!ds_state.IsUpdateAfterBind() && need_descriptor_checks) {
            ssbo_memory->desc_sets[i].ds_type = ds_state.GetTypeAddress(gpuav, loc);
        }
        descriptor_command_binding.bound_descriptor_sets.emplace_back(std::move(bound_descriptor_set));
    }
    cb_state.descriptor_command_bindings.emplace_back(std::move(descriptor_command_binding));

    descriptor_command_binding.ssbo_block.UnmapMemory();
}

// For the given command buffer, map its debug data buffers and update the status of any update after bind descriptors
[[nodiscard]] bool UpdateDescriptorStateSSBO(Validator &gpuav, CommandBuffer &cb_state, const Location &loc) {
    const bool need_descriptor_checks = gpuav.gpuav_settings.shader_instrumentation.bindless_descriptor;
    if (!need_descriptor_checks) return true;

    for (auto &descriptor_command_binding : cb_state.descriptor_command_bindings) {
        auto ssbo_memory = (glsl::DescriptorStateSSBO *)descriptor_command_binding.ssbo_block.MapMemory(loc);
        for (size_t i = 0; i < descriptor_command_binding.bound_descriptor_sets.size(); i++) {
            DescriptorSet &ds_state = *descriptor_command_binding.bound_descriptor_sets[i].state;
            ssbo_memory->desc_sets[i].ds_type = ds_state.GetTypeAddress(gpuav, loc);
        }
        descriptor_command_binding.ssbo_block.UnmapMemory();
    }
    return true;
}
}  // namespace descriptor

// After the GPU executed, we know which descriptor indexes were accessed and can validate with normal Core Validation logic
[[nodiscard]] bool CommandBuffer::ValidateBindlessDescriptorSets(const Location &loc) {
    // For each vkCmdBindDescriptorSets()...
    // Some applications repeatedly call vkCmdBindDescriptorSets() with the same descriptor sets, avoid
    // checking them multiple times.
    vvl::unordered_set<VkDescriptorSet> validated_desc_sets;
    for (auto [command_binding_index, descriptor_command_binding] : vvl::enumerate(descriptor_command_bindings)) {
        // TODO - Currently we don't know the actual call that triggered this, but without just giving "vkCmdDraw" we will get
        // VUID_Undefined
        Location draw_loc(vvl::Func::vkCmdDraw);

        // For each descriptor set ...
        for (uint32_t i = 0; i < descriptor_command_binding->bound_descriptor_sets.size(); i++) {
            auto &bound_descriptor_set = descriptor_command_binding->bound_descriptor_sets[i];
            if (validated_desc_sets.count(bound_descriptor_set.state->VkHandle()) > 0) {
                // TODO - If you share two VkDescriptorSet across two different sets in the SPIR-V, we are not going to be
                // validating the 2nd instance of it
                continue;
            }
            validated_desc_sets.emplace(bound_descriptor_set.state->VkHandle());
            if (!bound_descriptor_set.state->HasPostProcessBuffer()) {
                std::stringstream error;
                error << "In CommandBuffer::ValidateBindlessDescriptorSets, descriptor_command_binding[" << command_binding_index
                      << "].bound_descriptor_sets[" << i
                      << "].HasPostProcessBuffer() was false. This should not happen. GPU-AV is in a bad state, aborting.";
                auto gpuav = static_cast<Validator *>(&dev_data);
                gpuav->InternalError(gpuav->device, loc, error.str().c_str());
                return false;
            }

            vvl::DescriptorValidator context(state_, *this, *bound_descriptor_set.state, i, VK_NULL_HANDLE /*framebuffer*/,
                                             draw_loc);
            const uint32_t shader_set = glsl::kDescriptorSetWrittenMask | i;
            auto used_descs = bound_descriptor_set.state->UsedDescriptors(loc, shader_set);
            // For each used binding ...
            for (const auto &u : used_descs) {
                auto iter = bound_descriptor_set.binding_req_map.find(u.first);
                vvl::DescriptorBindingInfo binding_info;
                binding_info.first = u.first;
                while (iter != bound_descriptor_set.binding_req_map.end() && iter->first == u.first) {
                    binding_info.second.emplace_back(iter->second);
                    ++iter;
                }
                context.ValidateBinding(binding_info, u.second);
            }
        }
    }

    return true;
}
}  // namespace gpuav
