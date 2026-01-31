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

#include "drawdispatch/descriptor_validator.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_constants.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/shader_object_state.h"

#include "profiling/profiling.h"

namespace gpuav {

struct PostProcessingCbState {
    vko::BufferRange last_desc_set_binding_to_post_process_buffers_lut;
};

void RegisterPostProcessingValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.post_process_descriptor_indexing) {
        return;
    }

    DescriptorSetBindings& desc_set_bindings = cb.shared_resources_cache.GetOrCreate<DescriptorSetBindings>();

    desc_set_bindings.on_update_bound_descriptor_sets.emplace_back(
        [](Validator&, CommandBufferSubState& cb, DescriptorSetBindings::BindingCommand& desc_binding_cmd) {
            PostProcessingCbState& pp_cb_state = cb.shared_resources_cache.GetOrCreate<PostProcessingCbState>();

            pp_cb_state.last_desc_set_binding_to_post_process_buffers_lut =
                cb.gpu_resources_manager.GetDeviceLocalBufferRange(sizeof(glsl::PostProcessSSBO));

            desc_binding_cmd.desc_set_binding_to_post_process_buffers_lut =
                pp_cb_state.last_desc_set_binding_to_post_process_buffers_lut;
        });

    cb.on_instrumentation_desc_set_update_functions.emplace_back(
        [dummy_buffer_range = vko::BufferRange{}](CommandBufferSubState& cb, VkPipelineBindPoint, const Location&,
                                                  VkDescriptorBufferInfo& out_buffer_info, uint32_t& out_dst_binding) mutable {
            PostProcessingCbState* pp_cb_state = cb.shared_resources_cache.TryGet<PostProcessingCbState>();
            if (pp_cb_state) {
                out_buffer_info.buffer = pp_cb_state->last_desc_set_binding_to_post_process_buffers_lut.buffer;
                out_buffer_info.offset = pp_cb_state->last_desc_set_binding_to_post_process_buffers_lut.offset;
                out_buffer_info.range = pp_cb_state->last_desc_set_binding_to_post_process_buffers_lut.size;
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

            out_dst_binding = glsl::kBindingInstPostProcess;
        });

    auto bound_desc_sets_to_pp_buffer_map =
        std::make_shared<vvl::unordered_map<std::shared_ptr<vvl::DescriptorSet>, vko::StagingBuffer>>();
    cb.on_pre_cb_submission_functions.emplace_back([bound_desc_sets_to_pp_buffer_map](Validator& gpuav, CommandBufferSubState& cb,
                                                                                      VkCommandBuffer per_pre_submission_cb) {
        VVL_ZoneScoped;
        DescriptorSetBindings& desc_set_bindings = cb.shared_resources_cache.Get<DescriptorSetBindings>();

        for (const DescriptorSetBindings::BindingCommand& desc_binding_cmd : desc_set_bindings.descriptor_set_binding_commands) {
            vko::BufferRange desc_set_buffer_lut_buffer_range = cb.gpu_resources_manager.GetHostCoherentBufferRange(
                32 * sizeof(VkDeviceAddress));  // No driver offers more than 32 descriptor set bindings

            // For each unique bound descriptor set in this command buffer,
            // create an appropriate post processing buffer,
            // and update the "per CB submission descriptor set to post process buffers" LUT

            // For each CB submission, and for each descriptor binding command,
            // a "descriptor set to post process buffers LUT" is allocated and updated in a VkBuffer.
            // When executing, this CB submission will access its own private
            // post processing buffers, preventing concurrent use by another CB
            for (size_t ds_i = 0; ds_i < desc_binding_cmd.bound_descriptor_sets.size(); ds_i++) {
                // Perfectly can have gaps in descriptor sets bindings
                if (!desc_binding_cmd.bound_descriptor_sets[ds_i]) {
                    continue;
                }
                DescriptorSetSubState& desc_set_state = SubState(*desc_binding_cmd.bound_descriptor_sets[ds_i]);

                if (auto found = bound_desc_sets_to_pp_buffer_map->find(desc_binding_cmd.bound_descriptor_sets[ds_i]);
                    found == bound_desc_sets_to_pp_buffer_map->end()) {
                    // DescriptorSetSubState::GetPostProcessBufferSize() used to do a "auto guard = Lock()"
                    // But the lock was only guarding against GPU-AV sub state, not the base state, so
                    // base.GetNonInlineDescriptorCount() access were not fully protected
                    const VkDeviceSize pp_buffer_size =
                        desc_set_state.base.GetNonInlineDescriptorCount() * sizeof(glsl::PostProcessDescriptorIndexSlot);

                    if (pp_buffer_size == 0) {
                        continue;
                    }

                    vko::StagingBuffer staging_buffer(cb.gpu_resources_manager, pp_buffer_size, per_pre_submission_cb);

                    auto desc_set_buffer_lut_ptr = (VkDeviceAddress*)desc_set_buffer_lut_buffer_range.offset_mapped_ptr;
                    desc_set_buffer_lut_ptr[ds_i] = staging_buffer.GetBufferRange().offset_address;
                    bound_desc_sets_to_pp_buffer_map->insert({desc_binding_cmd.bound_descriptor_sets[ds_i], staging_buffer});
                } else {
                    auto desc_set_buffer_lut_ptr = (VkDeviceAddress*)desc_set_buffer_lut_buffer_range.offset_mapped_ptr;
                    desc_set_buffer_lut_ptr[ds_i] = found->second.GetBufferRange().offset_address;
                }
            }

            vko::CmdSynchronizedCopyBufferRange(per_pre_submission_cb,
                                                desc_binding_cmd.desc_set_binding_to_post_process_buffers_lut,
                                                desc_set_buffer_lut_buffer_range);
        }
    });

    if (vko::StagingBuffer::CanDeviceEverStage(gpuav)) {
        cb.on_post_cb_submission_functions.emplace_back([bound_desc_sets_to_pp_buffer_map](Validator& gpuav,
                                                                                           CommandBufferSubState& cb,
                                                                                           VkCommandBuffer per_post_submission_cb) {
            for (const auto& [desc_set, staging_buffer] : *bound_desc_sets_to_pp_buffer_map) {
                staging_buffer.CmdCopyDeviceToHost(per_post_submission_cb);
            }
        });
    }

    // Validate descriptor set accesses done by command buffer submission
    cb.on_cb_completion_functions.emplace_back([bound_desc_sets_to_pp_buffer_map](
                                                   Validator& gpuav, CommandBufferSubState& cb,
                                                   const CommandBufferSubState::LabelLogging& label_logging,
                                                   const Location& submission_loc) {
        VVL_ZoneScoped;

        // We loop each vkCmdBindDescriptorSet, find each VkDescriptorSet that was used in the command buffer, and check
        // its post process buffer for which descriptor was accessed Only check a VkDescriptorSet once, might be bound
        // multiple times in a single command buffer
        for (auto& [desc_set, staging_buffer] : *bound_desc_sets_to_pp_buffer_map) {
            // We build once here, but will update the set_index and shader_handle when found
            vvl::DescriptorValidator context(gpuav, cb.base, *desc_set, 0, VK_NULL_HANDLE, nullptr, Location(vvl::Func::Empty));

            // We create a map with the |unique_shader_id| as the key so we can only do the state object lookup once per
            // pipeline/shaderModule/shaderObject
            using DescriptorAccessMap = vvl::unordered_map<uint32_t, std::vector<DescriptorAccess>>;
            DescriptorAccessMap descriptor_access_map;
            {
                auto slot_ptr = (glsl::PostProcessDescriptorIndexSlot*)staging_buffer.GetHostBufferPtr();

                const std::vector<gpuav::spirv::BindingLayout>& binding_layouts = SubState(*desc_set).GetBindingLayouts();
                for (uint32_t binding = 0; binding < binding_layouts.size(); binding++) {
                    const gpuav::spirv::BindingLayout& binding_layout = binding_layouts[binding];
                    for (uint32_t descriptor_i = 0; descriptor_i < binding_layout.count; descriptor_i++) {
                        const glsl::PostProcessDescriptorIndexSlot slot = slot_ptr[binding_layout.start + descriptor_i];
                        if (slot.meta_data & glsl::kPostProcessMetaMaskAccessed) {
                            const uint32_t shader_id = slot.meta_data & glsl::kShaderIdMask;
                            const uint32_t error_logger_i = (slot.meta_data & glsl::kPostProcessMetaMaskErrorLoggerIndex) >>
                                                            glsl::kPostProcessMetaShiftErrorLoggerIndex;
                            descriptor_access_map[shader_id].emplace_back(DescriptorAccess{
                                binding, descriptor_i, slot.variable_id, slot.instruction_position_offset, error_logger_i});
                        }
                    }
                }
            }

            // For each shader ID we can do the state object lookup once, then validate all the accesses inside of it
            for (const auto& [shader_id, descriptor_accesses] : descriptor_access_map) {
                auto it = gpuav.instrumented_shaders_map_.find(shader_id);
                if (it == gpuav.instrumented_shaders_map_.end()) {
                    assert(false);
                    continue;
                }

                const vvl::Pipeline* pipeline_state = nullptr;
                const vvl::ShaderObject* shader_object_state = nullptr;

                if (it->second.pipeline != VK_NULL_HANDLE) {
                    // We use pipeline over vkShaderModule as likely they will have been destroyed by now
                    pipeline_state = gpuav.Get<vvl::Pipeline>(it->second.pipeline).get();
                } else if (it->second.shader_object != VK_NULL_HANDLE) {
                    shader_object_state = gpuav.Get<vvl::ShaderObject>(it->second.shader_object).get();
                    ASSERT_AND_CONTINUE(shader_object_state->entrypoint);
                } else {
                    assert(false);
                    continue;
                }

                context.SetOriginalSpirv(&it->second.original_spirv);

                uint32_t invalid_index_command = gpuav.gpuav_settings.GetInvalidIndexCommand();
                for (const DescriptorAccess& descriptor_access : descriptor_accesses) {
                    if (descriptor_access.error_logger_i == invalid_index_command) {
                        gpuav.LogError("GPUAV-Overflow-Unknown", LogObjectList(), Location(vvl::Func::Empty),
                                       "Cannot perform runtime descriptor access validation, access was done in a command past the "
                                       "internal limit of %" PRIu32 " draw/dispatch/traceRays in a command buffer.",
                                       gpuav.gpuav_settings.indices_count);
                        continue;
                    }

                    auto descriptor_binding = desc_set->GetBinding(descriptor_access.binding);
                    ASSERT_AND_CONTINUE(descriptor_binding);

                    const ::spirv::ResourceInterfaceVariable* resource_variable = nullptr;
                    if (pipeline_state) {
                        for (const ShaderStageState& stage_state : pipeline_state->stage_states) {
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

                    // If we already validated/updated the descriptor on the CPU, don't redo it now in GPU-AV Post
                    // Processing
                    if (!desc_set->ValidateBindingOnGPU(*descriptor_binding, *resource_variable)) {
                        continue;
                    }

                    context.SetInstructionPositionOffset(descriptor_access.instruction_position_offset);

                    // This will represent the Set that was accessed in the shader, which might not match the
                    // vkCmdBindDescriptorSet index if sets are aliased
                    context.SetSetIndexForGpuAv(resource_variable->decorations.set);

                    const CommandBufferSubState::CommandErrorLogger& cmd_error_logger =
                        cb.GetErrorLogger(descriptor_access.error_logger_i);
                    context.SetObjlistForGpuAv(&cmd_error_logger.objlist);
                    std::string debug_region_name =
                        cb.GetDebugLabelRegion(cmd_error_logger.label_cmd_i, label_logging.initial_label_stack);

                    Location access_loc(cmd_error_logger.loc.Get(), debug_region_name);
                    context.SetLocationForGpuAv(access_loc);
                    context.ValidateBindingDynamic(*resource_variable, *descriptor_binding, descriptor_access.index);
                }
            }
        }

        return true;
    });
}
}  // namespace gpuav