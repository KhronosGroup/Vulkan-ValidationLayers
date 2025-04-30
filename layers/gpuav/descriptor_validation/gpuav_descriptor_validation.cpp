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
#include "utils/vk_layer_utils.h"

namespace gpuav {
namespace descriptor {

void UpdateBoundDescriptorsPostProcess(Validator &gpuav, CommandBufferSubState &cb_state, const LastBound &last_bound,
                                       DescriptorBindingCommand &descriptor_binding_cmd) {
    if (!gpuav.gpuav_settings.shader_instrumentation.post_process_descriptor_indexing) return;

    // Create a new buffer to hold our BDA pointers
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = sizeof(glsl::PostProcessSSBO);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.pool = VK_NULL_HANDLE;
    const bool success = descriptor_binding_cmd.post_process_ssbo_buffer.Create(&buffer_info, &alloc_info);
    if (!success) {
        return;
    }

    descriptor_binding_cmd.post_process_ssbo_buffer.Clear();
    auto ssbo_buffer_ptr = (glsl::PostProcessSSBO *)descriptor_binding_cmd.post_process_ssbo_buffer.GetMappedPtr();

    cb_state.post_process_buffer_lut = descriptor_binding_cmd.post_process_ssbo_buffer.VkHandle();

    const size_t number_of_sets = last_bound.ds_slots.size();
    for (uint32_t i = 0; i < number_of_sets; i++) {
        const auto &ds_slot = last_bound.ds_slots[i];
        if (!ds_slot.ds_state) {
            continue;  // can have gaps in descriptor sets
        }

        ssbo_buffer_ptr->descriptor_index_post_process_buffers[i] = SubState(*ds_slot.ds_state).GetPostProcessBuffer(gpuav);
    }
}

void UpdateBoundDescriptorsDescriptorChecks(Validator &gpuav, CommandBufferSubState &cb_state, const LastBound &last_bound,
                                            DescriptorBindingCommand &descriptor_binding_cmd, const Location &loc) {
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

    DescriptorBindingCommand descriptor_binding_cmd(gpuav);
    descriptor_binding_cmd.bound_descriptor_sets.reserve(number_of_sets);
    // Currently we loop through the sets multiple times to reduce complexity and seperate the various parts, can revisit if we find
    // this is actually a perf bottleneck (assume number of sets are low as people we will then to have a single large set)
    for (uint32_t i = 0; i < number_of_sets; i++) {
        const auto &ds_slot = last_bound.ds_slots[i];
        descriptor_binding_cmd.bound_descriptor_sets.emplace_back(ds_slot.ds_state);
    }

    UpdateBoundDescriptorsPostProcess(gpuav, cb_state, last_bound, descriptor_binding_cmd);
    UpdateBoundDescriptorsDescriptorChecks(gpuav, cb_state, last_bound, descriptor_binding_cmd, loc);

    cb_state.descriptor_binding_commands.emplace_back(std::move(descriptor_binding_cmd));
}

// For the given command buffer, map its debug data buffers and update the status of any update after bind descriptors
[[nodiscard]] bool UpdateDescriptorStateSSBO(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc) {
    const bool need_descriptor_checks = gpuav.gpuav_settings.shader_instrumentation.descriptor_checks;
    if (!need_descriptor_checks) return true;

    for (auto &descriptor_binding_cmd : cb_state.descriptor_binding_commands) {
        auto ssbo_buffer_ptr = (glsl::DescriptorStateSSBO *)descriptor_binding_cmd.descritpor_state_ssbo_buffer.GetMappedPtr();
        for (size_t i = 0; i < descriptor_binding_cmd.bound_descriptor_sets.size(); i++) {
            auto &bound_desc_set = descriptor_binding_cmd.bound_descriptor_sets[i];
            // Some descriptor set slots may be unbound
            if (!bound_desc_set) {
                continue;
            }
            auto &substate = SubState(*bound_desc_set);
            ssbo_buffer_ptr->descriptor_set_types[i] = substate.GetTypeAddress(gpuav, loc);
        }
    }
    return true;
}
}  // namespace descriptor

// After the GPU executed, we know which descriptor indexes were accessed and can validate with normal Core Validation logic
[[nodiscard]] bool CommandBufferSubState::ValidateBindlessDescriptorSets(const Location &loc, const LabelLogging &label_logging) {
    VVL_ZoneScoped;
    // Only check a VkDescriptorSet once, might be bound multiple times in a single command buffer
    vvl::unordered_set<VkDescriptorSet> validated_desc_sets;

    // TODO - Currently we don't know the actual call that triggered this, but without just giving "vkCmdDraw" we will get
    // VUID_Undefined We now have the DescriptorValidator::action_index, just need to hook it up!
    Location draw_loc(vvl::Func::vkCmdDraw);

    // We loop each vkCmdBindDescriptorSet, find each VkDescriptorSet that was used in the command buffer, and check its post
    // process buffer for which descriptor was accessed
    for (const auto &descriptor_binding_cmd : descriptor_binding_commands) {
        for (uint32_t set_index = 0; set_index < descriptor_binding_cmd.bound_descriptor_sets.size(); set_index++) {
            auto &bound_desc_set = descriptor_binding_cmd.bound_descriptor_sets[set_index];
            // Some descriptor set slots may be unbound
            if (!bound_desc_set) {
                continue;
            }
            auto &ds_sub_state = SubState(*bound_desc_set);

            // The Post Process buffer is tied to the VkDescriptorSet, so we clear it after and only check it once
            if (validated_desc_sets.count(bound_desc_set->VkHandle()) > 0) {
                // TODO - If you share two VkDescriptorSet across two different sets in the SPIR-V, we are not going to be
                // validating the 2nd instance of it
                continue;
            }

            if (!ds_sub_state.HasPostProcessBuffer()) {
                if (!ds_sub_state.CanPostProcess()) {
                    continue;  // hit a dummy object used as a placeholder
                } else if (bound_desc_set->GetBindingCount() == 0) {
                    continue;  // empty set
                }

                std::stringstream error;
                error << "In CommandBuffer::ValidateBindlessDescriptorSets, descriptor_binding_cmd.bound_descriptor_sets["
                      << set_index
                      << "].HasPostProcessBuffer() was false. This should not happen. GPU-AV is in a bad state, aborting.";

                gpuav_.InternalError(gpuav_.device, loc, error.str().c_str());
                return false;
            }
            validated_desc_sets.emplace(bound_desc_set->VkHandle());

            // We build once here, but will update the set_index and shader_handle when found
            vvl::DescriptorValidator context(gpuav_, base, *bound_desc_set, 0, VK_NULL_HANDLE, nullptr, draw_loc);

            DescriptorAccessMap descriptor_access_map = ds_sub_state.GetDescriptorAccesses(loc);
            // Once we have accessed everything and created the DescriptorAccess, we can clear this buffer
            ds_sub_state.ClearPostProcess();

            // For each shader ID we can do the state object lookup once, then validate all the accesses inside of it
            for (const auto &[shader_id, descriptor_accesses] : descriptor_access_map) {
                auto it = gpuav_.instrumented_shaders_map_.find(shader_id);
                if (it == gpuav_.instrumented_shaders_map_.end()) {
                    assert(false);
                    continue;
                }

                const vvl::Pipeline *pipeline_state = nullptr;
                const vvl::ShaderObject *shader_object_state = nullptr;

                if (it->second.pipeline != VK_NULL_HANDLE) {
                    // We use pipeline over vkShaderModule as likely they will have been destroyed by now
                    pipeline_state = gpuav_.Get<vvl::Pipeline>(it->second.pipeline).get();
                    context.SetShaderHandleForGpuAv(&pipeline_state->Handle());
                } else if (it->second.shader_object != VK_NULL_HANDLE) {
                    shader_object_state = gpuav_.Get<vvl::ShaderObject>(it->second.shader_object).get();
                    ASSERT_AND_CONTINUE(shader_object_state->entrypoint);
                    context.SetShaderHandleForGpuAv(&shader_object_state->Handle());
                } else {
                    assert(false);
                    continue;
                }

                for (const auto &descriptor_access : descriptor_accesses) {
                    auto descriptor_binding = bound_desc_set->GetBinding(descriptor_access.binding);
                    ASSERT_AND_CONTINUE(descriptor_binding);

                    const ::spirv::ResourceInterfaceVariable *resource_variable = nullptr;
                    if (pipeline_state) {
                        for (const auto &stage_state : pipeline_state->stage_states) {
                            ASSERT_AND_CONTINUE(stage_state.entrypoint);
                            auto variable_it =
                                stage_state.entrypoint->resource_interface_variable_map.find(descriptor_access.variable_id);
                            if (variable_it != stage_state.entrypoint->resource_interface_variable_map.end()) {
                                resource_variable = variable_it->second;
                                break;  // Only need to find a single entry point
                            }
                        }
                    } else if (shader_object_state) {
                        ASSERT_AND_CONTINUE(shader_object_state->entrypoint);
                        auto variable_it =
                            shader_object_state->entrypoint->resource_interface_variable_map.find(descriptor_access.variable_id);
                        if (variable_it != shader_object_state->entrypoint->resource_interface_variable_map.end()) {
                            resource_variable = variable_it->second;
                        }
                    }
                    ASSERT_AND_CONTINUE(resource_variable);

                    // If we already validated/updated the descriptor on the CPU, don't redo it now in GPU-AV Post Processing
                    if (!bound_desc_set->ValidateBindingOnGPU(*descriptor_binding, *resource_variable)) {
                        continue;
                    }

                    // This will represent the Set that was accessed in the shader, which might not match the vkCmdBindDescriptorSet
                    // index if sets are aliased
                    context.SetSetIndexForGpuAv(resource_variable->decorations.set);

                    std::string debug_region_name;
                    if (auto found_label_cmd_i = label_logging.action_cmd_i_to_label_cmd_i_map.find(descriptor_access.action_index);
                        found_label_cmd_i != label_logging.action_cmd_i_to_label_cmd_i_map.end()) {
                        debug_region_name = GetDebugLabelRegion(found_label_cmd_i->second, label_logging.initial_label_stack);
                    }

                    Location access_loc(loc, debug_region_name);
                    context.SetLocationForGpuAv(access_loc);
                    context.ValidateBindingDynamic(*resource_variable, *descriptor_binding, descriptor_access.index);
                }
            }
        }
    }

    return true;
}
}  // namespace gpuav
