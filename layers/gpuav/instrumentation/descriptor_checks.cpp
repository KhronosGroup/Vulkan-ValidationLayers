/* Copyright (c) 2024-2025 LunarG, Inc.
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

#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/resources/gpuav_state_trackers.h"

namespace gpuav {

struct DescriptorChecksCbState {
    vko::BufferRange last_bound_desc_sets_state_ssbo;
};

void RegisterDescriptorChecksValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        return;
    }

    DescriptorSetBindings& desc_set_bindings = cb.shared_resources_cache.GetOrCreate<DescriptorSetBindings>();

    desc_set_bindings.on_update_bound_descriptor_sets.emplace_back(
        [](Validator& gpuav, CommandBufferSubState& cb, DescriptorSetBindings::BindingCommand& desc_binding_cmd) {
            DescriptorChecksCbState& desc_checks_cb_state = cb.shared_resources_cache.GetOrCreate<DescriptorChecksCbState>();
            desc_checks_cb_state.last_bound_desc_sets_state_ssbo =
                cb.gpu_resources_manager.GetHostVisibleBufferRange(sizeof(glsl::BoundDescriptorSetsStateSSBO));
            memset(desc_checks_cb_state.last_bound_desc_sets_state_ssbo.offset_mapped_ptr, 0,
                   size_t(desc_checks_cb_state.last_bound_desc_sets_state_ssbo.size));
            auto desc_state_ssbo = static_cast<glsl::BoundDescriptorSetsStateSSBO*>(
                desc_checks_cb_state.last_bound_desc_sets_state_ssbo.offset_mapped_ptr);
            desc_state_ssbo->descriptor_init_status = gpuav.desc_heap_->GetDeviceAddress();

            for (size_t bound_ds_i = 0; bound_ds_i < desc_binding_cmd.bound_descriptor_sets.size(); ++bound_ds_i) {
                auto& bound_ds = desc_binding_cmd.bound_descriptor_sets[bound_ds_i];
                // Account for gaps in descriptor sets bindings
                if (!bound_ds) {
                    continue;
                }

                if (bound_ds->IsUpdateAfterBind()) {
                    continue;
                }

                desc_state_ssbo->descriptor_set_types[bound_ds_i] =
                    SubState(*bound_ds).GetTypeAddress(gpuav, Location(vvl::Func::Empty));
            }

            desc_binding_cmd.descritpor_state_ssbo = desc_checks_cb_state.last_bound_desc_sets_state_ssbo;
        });

    cb.on_instrumentation_desc_set_update_functions.emplace_back(
        [dummy_buffer_range = vko::BufferRange{}](CommandBufferSubState& cb, VkDescriptorBufferInfo& out_buffer_info,
                                                  uint32_t& out_dst_binding) mutable {
            DescriptorChecksCbState* desc_checks_cb_state = cb.shared_resources_cache.TryGet<DescriptorChecksCbState>();
            if (desc_checks_cb_state) {
                out_buffer_info.buffer = desc_checks_cb_state->last_bound_desc_sets_state_ssbo.buffer;
                out_buffer_info.offset = desc_checks_cb_state->last_bound_desc_sets_state_ssbo.offset;
                out_buffer_info.range = desc_checks_cb_state->last_bound_desc_sets_state_ssbo.size;
            } else {
                // Eventually, no descriptor set was bound in command buffer.
                // Instrumenation descriptor set is already defined at this point and needs a binding,
                // so just provide a dummy buffer
                if (dummy_buffer_range.buffer == VK_NULL_HANDLE) {
                    dummy_buffer_range = cb.gpu_resources_manager.GetDeviceLocalBufferRange(64);
                }
                out_buffer_info.buffer = dummy_buffer_range.buffer;
                out_buffer_info.offset = dummy_buffer_range.offset;
                out_buffer_info.range = dummy_buffer_range.size;
            }

            out_dst_binding = glsl::kBindingInstDescriptorIndexingOOB;
        });

    // For every descriptor binding command, update a GPU buffer holding the type of each bound descriptor set
    cb.on_pre_cb_submission_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb, VkCommandBuffer) {
        DescriptorSetBindings& desc_set_bindings = cb.shared_resources_cache.Get<DescriptorSetBindings>();
        for (DescriptorSetBindings::BindingCommand& desc_binding_cmd : desc_set_bindings.descriptor_set_binding_commands) {
            auto desc_state_ssbo_ptr =
                static_cast<glsl::BoundDescriptorSetsStateSSBO*>(desc_binding_cmd.descritpor_state_ssbo.offset_mapped_ptr);
            for (size_t bound_ds_i = 0; bound_ds_i < desc_binding_cmd.bound_descriptor_sets.size(); ++bound_ds_i) {
                auto& bound_ds = desc_binding_cmd.bound_descriptor_sets[bound_ds_i];
                // Account for gaps in descriptor sets bindings
                if (!bound_ds) {
                    continue;
                }
                DescriptorSetSubState& desc_set_state = SubState(*bound_ds);
                desc_state_ssbo_ptr->descriptor_set_types[bound_ds_i] =
                    desc_set_state.GetTypeAddress(gpuav, Location(vvl::Func::Empty));
            }
        }
    });
}

}  // namespace gpuav
