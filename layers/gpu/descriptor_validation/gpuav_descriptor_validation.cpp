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
    if (!gpuav.gpuav_settings.shader_instrumentation.bindless_descriptor &&
        !gpuav.gpuav_settings.shader_instrumentation.post_process_descriptor_index) {
        return;
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];
    // Should have just been updated
    if (!last_bound.pipeline_state) {
        gpuav.InternalError(pipeline, loc, "Unrecognized pipeline");
        return;
    }

    // Catch if pipeline is bound before any descriptor sets
    if (cb_state.di_input_buffer_list.empty()) {
        return;
    }

    // Update the last vkCmdBindDescriptorSet with the new pipeline
    auto &descriptor_set_buffers = cb_state.di_input_buffer_list.back().descriptor_set_buffers;

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
                descriptor_set_buffers[update_index++].binding_req_map = slot->second;
            }
        }
    }
}

void UpdateBoundDescriptors(Validator &gpuav, CommandBuffer &cb_state, VkPipelineBindPoint pipeline_bind_point,
                            const Location &loc) {
    if (!gpuav.gpuav_settings.shader_instrumentation.bindless_descriptor &&
        !gpuav.gpuav_settings.shader_instrumentation.post_process_descriptor_index) {
        return;
    }

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
    buffer_info.size = sizeof(glsl::BindlessStateBuffer);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.pool = VK_NULL_HANDLE;
    DescBindingInfo di_buffers(gpuav);

    // Allocate buffer for device addresses of the input buffer for each descriptor set.  This is the buffer written to each
    // draw's descriptor set.
    di_buffers.bindless_state.CreateBuffer(loc, &buffer_info, &alloc_info);

    auto bindless_state = (glsl::BindlessStateBuffer *)di_buffers.bindless_state.MapMemory(loc);

    memset(bindless_state, 0, static_cast<size_t>(buffer_info.size));
    cb_state.current_bindless_buffer = di_buffers.bindless_state.Buffer();

    bindless_state->global_state = gpuav.desc_heap_->GetDeviceAddress();
    di_buffers.descriptor_set_buffers.reserve(di_buffers.descriptor_set_buffers.size() + last_bound.per_set.size());
    for (uint32_t i = 0; i < last_bound.per_set.size(); i++) {
        const auto &last_bound_set = last_bound.per_set[i];
        if (!last_bound_set.bound_descriptor_set) {
            continue;
        }

        DescSetState desc_set_state;
        desc_set_state.state = std::static_pointer_cast<DescriptorSet>(last_bound_set.bound_descriptor_set);
        bindless_state->desc_sets[i].descriptor_index_lut = desc_set_state.state->GetIndexLUTAddress(gpuav, loc);
        // The pipeline might not have been bound yet, so will need to update binding_req_map later
        if (last_bound.pipeline_state) {
            auto slot = last_bound.pipeline_state->active_slots.find(i);
            if (slot != last_bound.pipeline_state->active_slots.end()) {
                desc_set_state.binding_req_map = slot->second;
            }
        }
        if (!desc_set_state.state->IsUpdateAfterBind()) {
            bindless_state->desc_sets[i].ds_type = desc_set_state.state->GetTypeAddress(gpuav, loc);
            bindless_state->desc_sets[i].descriptor_index_post_process = desc_set_state.state->GetPostProcessBuffer(gpuav, loc);
        }
        di_buffers.descriptor_set_buffers.emplace_back(std::move(desc_set_state));
    }
    cb_state.di_input_buffer_list.emplace_back(std::move(di_buffers));

    di_buffers.bindless_state.UnmapMemory();
}

// For the given command buffer, map its debug data buffers and update the status of any update after bind descriptors
[[nodiscard]] bool UpdateBindlessStateBuffer(Validator &gpuav, CommandBuffer &cb_state, const Location &loc) {
    for (auto &cmd_info : cb_state.di_input_buffer_list) {
        auto bindless_state = (glsl::BindlessStateBuffer *)cmd_info.bindless_state.MapMemory(loc);

        for (size_t i = 0; i < cmd_info.descriptor_set_buffers.size(); i++) {
            auto &set_buffer = cmd_info.descriptor_set_buffers[i];
            bindless_state->desc_sets[i].descriptor_index_lut = set_buffer.state->GetIndexLUTAddress(gpuav, loc);
            bindless_state->desc_sets[i].ds_type = set_buffer.state->GetTypeAddress(gpuav, loc);
            bindless_state->desc_sets[i].descriptor_index_post_process = set_buffer.state->GetPostProcessBuffer(gpuav, loc);
        }
        cmd_info.bindless_state.UnmapMemory();
    }
    return true;
}
}  // namespace descriptor

static std::map<uint32_t, std::vector<uint32_t>> UsedDescriptors(const Location &loc, const DeviceMemoryBlock &layout,
                                                                 const DeviceMemoryBlock &output_state, uint32_t shader_set) {
    std::map<uint32_t, std::vector<uint32_t>> used_descriptors;
    if (output_state.Destroyed()) {
        return used_descriptors;
    }

    auto layout_data = (glsl::BindingLayout *)layout.MapMemory(loc);

    auto data = (uint32_t *)output_state.MapMemory(loc);
    output_state.InvalidateAllocation(loc);

    uint32_t max_binding = layout_data[0].count;
    for (uint32_t binding = 0; binding < max_binding; binding++) {
        uint32_t count = layout_data[binding + 1].count;
        uint32_t start = layout_data[binding + 1].state_start;
        for (uint32_t i = 0; i < count; i++) {
            uint32_t pos = start + i;
            if (data[pos] == shader_set) {
                auto map_result = used_descriptors.emplace(binding, std::vector<uint32_t>());
                map_result.first->second.emplace_back(i);
            }
        }
    }

    output_state.UnmapMemory();
    layout.UnmapMemory();
    return used_descriptors;
}

// After the GPU executed, we know which descriptor indexes were accessed and can validate with normal Core Validation logic
[[nodiscard]] bool CommandBuffer::ValidateBindlessDescriptorSets(const Location &loc) {
    // For each vkCmdBindDescriptorSets()...
    // Some applications repeatedly call vkCmdBindDescriptorSets() with the same descriptor sets, avoid
    // checking them multiple times.
    vvl::unordered_set<VkDescriptorSet> validated_desc_sets;
    for (auto [di_info_i, di_info] : vvl::enumerate(di_input_buffer_list)) {
        // TODO - Currently we don't know the actual call that triggered this, but without just giving "vkCmdDraw" we will get
        // VUID_Undefined
        Location draw_loc(vvl::Func::vkCmdDraw);

        // For each descriptor set ...
        for (uint32_t i = 0; i < di_info->descriptor_set_buffers.size(); i++) {
            auto &set = di_info->descriptor_set_buffers[i];
            if (validated_desc_sets.count(set.state->VkHandle()) > 0) {
                // TODO - If you share two VkDescriptorSet across two different sets in the SPIR-V, we are not going to be
                // validating the 2nd instance of it
                continue;
            }
            validated_desc_sets.emplace(set.state->VkHandle());
            if (!set.state->HasPostProcessBuffer()) {
                std::stringstream error;
                error << "In CommandBuffer::ValidateBindlessDescriptorSets, di_info[" << di_info_i << "].descriptor_set_buffers["
                      << i << "].HasPostProcessBuffer() was false. This should not happen. GPU-AV is in a bad state, aborting.";
                auto gpuav = static_cast<Validator *>(&dev_data);
                gpuav->InternalError(gpuav->device, loc, error.str().c_str());
                return false;
            }

            vvl::DescriptorValidator context(state_, *this, *set.state, i, VK_NULL_HANDLE /*framebuffer*/, draw_loc);
            const uint32_t shader_set = glsl::kDescriptorSetWrittenMask | i;
            auto used_descs = UsedDescriptors(loc, set.state->LayoutBlock(), set.state->post_process_block_, shader_set);
            // For each used binding ...
            for (const auto &u : used_descs) {
                auto iter = set.binding_req_map.find(u.first);
                vvl::DescriptorBindingInfo binding_info;
                binding_info.first = u.first;
                while (iter != set.binding_req_map.end() && iter->first == u.first) {
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
