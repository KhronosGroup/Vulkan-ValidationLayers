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

namespace gpuav {
void UpdateBoundPipeline(Validator &gpuav, VkCommandBuffer cb, VkPipelineBindPoint pipeline_bind_point, VkPipeline pipeline,
                         const Location &loc) {
    if (!gpuav.gpuav_settings.validate_descriptors) return;
    auto cb_state = gpuav.GetWrite<CommandBuffer>(cb);
    if (!cb_state) {
        gpuav.InternalError(cb, loc, "Unrecognized command buffer");
        return;
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    auto const &last_bound = cb_state->lastBound[lv_bind_point];
    // Should have just been updated
    if (!last_bound.pipeline_state) {
        gpuav.InternalError(pipeline, loc, "Unrecognized pipeline");
        return;
    }

    // Catch if pipeline is bound before any descriptor sets
    if (cb_state->di_input_buffer_list.empty()) {
        return;
    }

    // Update the last vkCmdBindDescriptorSet with the new pipeline
    auto &descriptor_set_buffers = cb_state->di_input_buffer_list.back().descriptor_set_buffers;

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

void UpdateBoundDescriptors(Validator &gpuav, VkCommandBuffer cb, VkPipelineBindPoint pipeline_bind_point, const Location &loc) {
    auto cb_state = gpuav.GetWrite<CommandBuffer>(cb);
    if (!cb_state) {
        gpuav.InternalError(cb, loc, "Unrecognized command buffer");
        return;
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    auto const &last_bound = cb_state->lastBound[lv_bind_point];

    uint32_t number_of_sets = static_cast<uint32_t>(last_bound.per_set.size());

    if (number_of_sets == 0 || !gpuav.gpuav_settings.validate_descriptors || !gpuav.force_buffer_device_address_) {
        return;
    }

    // Figure out how much memory we need for the input block based on how many sets and bindings there are
    // and how big each of the bindings is

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
    VkResult result = vmaCreateBuffer(gpuav.vma_allocator_, &buffer_info, &alloc_info, &di_buffers.bindless_state_buffer,
                                      &di_buffers.bindless_state_buffer_allocation, nullptr);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(cb_state->Handle(), loc, "Unable to allocate device memory. Device could become unstable.", true);
        return;
    }
    glsl::BindlessStateBuffer *bindless_state{nullptr};
    result =
        vmaMapMemory(gpuav.vma_allocator_, di_buffers.bindless_state_buffer_allocation, reinterpret_cast<void **>(&bindless_state));
    if (result != VK_SUCCESS) {
        gpuav.InternalError(cb_state->Handle(), loc, "Unable to map device memory. Device could become unstable.", true);
        return;
    }
    memset(bindless_state, 0, static_cast<size_t>(buffer_info.size));
    cb_state->current_bindless_buffer = di_buffers.bindless_state_buffer;

    bindless_state->global_state = gpuav.desc_heap_->GetDeviceAddress();
    for (uint32_t i = 0; i < last_bound.per_set.size(); i++) {
        const auto &s = last_bound.per_set[i];
        auto set = s.bound_descriptor_set;
        if (!set) {
            continue;
        }
        if (gpuav.gpuav_settings.validate_descriptors) {
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
                desc_set_state.output_state = desc_set_state.state->GetOutputState(gpuav);
                if (!desc_set_state.output_state) {
                    goto exit;
                }
                bindless_state->desc_sets[i].out_data = desc_set_state.output_state->device_addr;
            }
            di_buffers.descriptor_set_buffers.emplace_back(std::move(desc_set_state));
        }
    }
    cb_state->di_input_buffer_list.emplace_back(di_buffers);
exit:
    vmaUnmapMemory(gpuav.vma_allocator_, di_buffers.bindless_state_buffer_allocation);
}

// For the given command buffer, map its debug data buffers and update the status of any update after bind descriptors
[[nodiscard]] bool UpdateBindlessStateBuffer(Validator &gpuav, CommandBuffer &cb_state, VmaAllocator vma_allocator) {
    for (auto &cmd_info : cb_state.di_input_buffer_list) {
        glsl::BindlessStateBuffer *bindless_state{nullptr};
        [[maybe_unused]] VkResult result;
        result = vmaMapMemory(vma_allocator, cmd_info.bindless_state_buffer_allocation, reinterpret_cast<void **>(&bindless_state));
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(gpuav.device, Location(vvl::Func::vkMapMemory),
                                "Unable to map device memory allocated for error output buffer. Aborting GPU-AV.", true);
            return false;
        }
        for (size_t i = 0; i < cmd_info.descriptor_set_buffers.size(); i++) {
            auto &set_buffer = cmd_info.descriptor_set_buffers[i];
            bindless_state->desc_sets[i].layout_data = set_buffer.state->GetLayoutState();
            if (!set_buffer.gpu_state) {
                set_buffer.gpu_state = set_buffer.state->GetCurrentState();
                bindless_state->desc_sets[i].in_data = set_buffer.gpu_state->device_addr;
            }
            if (!set_buffer.output_state) {
                set_buffer.output_state = set_buffer.state->GetOutputState(gpuav);
                if (!set_buffer.output_state) {
                    vmaUnmapMemory(vma_allocator, cmd_info.bindless_state_buffer_allocation);
                    return false;
                }
                bindless_state->desc_sets[i].out_data = set_buffer.output_state->device_addr;
            }
        }
        vmaUnmapMemory(vma_allocator, cmd_info.bindless_state_buffer_allocation);
    }
    return true;
}

[[nodiscard]] bool CommandBuffer::ValidateBindlessDescriptorSets() {
    // For each vkCmdBindDescriptorSets()...
    // Some applications repeatedly call vkCmdBindDescriptorSets() with the same descriptor sets, avoid
    // checking them multiple times.
    vvl::unordered_set<VkDescriptorSet> validated_desc_sets;
    for (auto [di_info_i, di_info] : vvl::enumerate(di_input_buffer_list)) {
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
            assert(set.output_state);
            if (!set.output_state) {
                std::stringstream error;
                error << "In CommandBuffer::ValidateBindlessDescriptorSets, di_info[" << di_info_i << "].descriptor_set_buffers["
                      << i << "].output_state was null. This should not happen. GPU-AV is in a bad state, aborting.";
                auto gpuav = static_cast<Validator *>(&dev_data);
                gpuav->InternalError(gpuav->device, Location(vvl::Func::vkQueueSubmit), error.str().c_str());
                return false;
            }

            vvl::DescriptorValidator context(state_, *this, *set.state, i, VK_NULL_HANDLE /*framebuffer*/, draw_loc);
            const uint32_t shader_set = glsl::kDescriptorSetWrittenMask | i;
            auto used_descs = set.output_state->UsedDescriptors(*set.state, shader_set);
            // For each used binding ...
            for (const auto &u : used_descs) {
                auto iter = set.binding_req.find(u.first);
                vvl::DescriptorBindingInfo binding_info;
                binding_info.first = u.first;
                while (iter != set.binding_req.end() && iter->first == u.first) {
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
