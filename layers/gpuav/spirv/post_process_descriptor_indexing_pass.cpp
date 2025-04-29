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

#include "post_process_descriptor_indexing_pass.h"
#include "module.h"
#include <iostream>

#include "drawdispatch/descriptor_validator.h"
#include "generated/gpuav_offline_spirv.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_validation_pipeline.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"

#include "state_tracker/pipeline_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/shader_object_state.h"
#include "utils/hash_util.h"

#include "profiling/profiling.h"

namespace gpuav {

void RegisterPostProcessingValidation(Validator& gpuav, CommandBufferSubState& cb) {
    CommandBufferDescriptorBindings& cb_desc_bindings = cb.shared_resources_cache.GetOrCreate<CommandBufferDescriptorBindings>();

    cb_desc_bindings.on_descriptor_binding_functions.emplace_back(
        [](CommandBufferSubState& cb, DescriptorBindingCommand& desc_binding_cmd) {
            PostProcessingCbState& pp_cb_state = cb.shared_resources_cache.GetOrCreate<PostProcessingCbState>();

            pp_cb_state.last_desc_set_binding_to_post_process_buffers_lut =
                cb.gpu_resources_manager.GetDeviceLocalBufferRange(sizeof(glsl::PostProcessSSBO));

            desc_binding_cmd.desc_set_binding_to_post_process_buffers_lut =
                pp_cb_state.last_desc_set_binding_to_post_process_buffers_lut;
        });

    cb.on_instrumentation_desc_set_update_functions.emplace_back(
        [dummy_buffer_range = vko::BufferRange{}](CommandBufferSubState& cb, VkDescriptorBufferInfo& out_buffer_info,
                                                  uint32_t& out_dst_binding) mutable {
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

    cb.on_cb_submission_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb, VkCommandBuffer per_submission_cb) {
        VVL_ZoneScoped;
        CommandBufferDescriptorBindings& cb_desc_bindings = cb.shared_resources_cache.Get<CommandBufferDescriptorBindings>();

        vvl::unordered_map<std::shared_ptr<vvl::DescriptorSet>, vko::BufferRange> bound_desc_sets_to_pp_buffer_map;
        for (const DescriptorBindingCommand& desc_binding_cmd : cb_desc_bindings.descriptor_binding_commands) {
            vko::BufferRange desc_set_buffer_lut_buffer_range =
                cb.gpu_resources_manager.GetHostVisibleBufferRange(32 * sizeof(VkDeviceAddress));

            // For each unique bound descriptor set in this command buffer,
            // create an appropriate post processing buffer,
            // and update the per CB submission desciptor set to post process buffers LUT

            // For each CB submission, and for each descriptor binding command,
            // a "desciptor set to post process buffers LUT" is allocated and updated in a VkBuffer.
            // When executing, this CB submission will access its own private
            // post processing buffers, preventing concurrent use by another CB
            for (size_t ds_i = 0; ds_i < desc_binding_cmd.bound_descriptor_sets.size(); ds_i++) {
                // Perfectly can have gaps in descriptor sets bindings
                if (!desc_binding_cmd.bound_descriptor_sets[ds_i]) {
                    continue;
                }
                DescriptorSetSubState& desc_set_state = SubState(*desc_binding_cmd.bound_descriptor_sets[ds_i]);

                if (auto found = bound_desc_sets_to_pp_buffer_map.find(desc_binding_cmd.bound_descriptor_sets[ds_i]);
                    found == bound_desc_sets_to_pp_buffer_map.end()) {
                    const VkDeviceSize pp_buffer_size = desc_set_state.GetPostProcessBufferSize();

                    if (pp_buffer_size == 0) {
                        continue;
                    }

                    vko::BufferRange pp_buffer_range = cb.gpu_resources_manager.GetHostCachedBufferRange(pp_buffer_size);
                    memset((std::byte*)pp_buffer_range.offset_mapped_ptr, 0, (size_t)pp_buffer_range.size);
                    cb.gpu_resources_manager.FlushAllocation(pp_buffer_range);

                    auto desc_set_buffer_lut_ptr = (VkDeviceAddress*)desc_set_buffer_lut_buffer_range.offset_mapped_ptr;
                    desc_set_buffer_lut_ptr[ds_i] = pp_buffer_range.offset_address;
                    bound_desc_sets_to_pp_buffer_map.insert({desc_binding_cmd.bound_descriptor_sets[ds_i], pp_buffer_range});
                } else {
                    auto desc_set_buffer_lut_ptr = (VkDeviceAddress*)desc_set_buffer_lut_buffer_range.offset_mapped_ptr;
                    desc_set_buffer_lut_ptr[ds_i] = found->second.offset_address;
                }
            }

            // Dispatch a copy command, copying the per CB submission descriptor set LUT to the LUT created at
            // "bind descriptor set command" record time, aka the one that shaders will ultimately access.
            {
                VkBufferMemoryBarrier barrier_write_after_read = vku::InitStructHelper();
                barrier_write_after_read.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                barrier_write_after_read.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier_write_after_read.buffer = desc_binding_cmd.desc_set_binding_to_post_process_buffers_lut.buffer;
                barrier_write_after_read.offset = desc_binding_cmd.desc_set_binding_to_post_process_buffers_lut.offset;
                barrier_write_after_read.size = desc_binding_cmd.desc_set_binding_to_post_process_buffers_lut.size;

                DispatchCmdPipelineBarrier(per_submission_cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                           VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1, &barrier_write_after_read, 0,
                                           nullptr);

                VkBufferCopy copy;
                copy.srcOffset = desc_set_buffer_lut_buffer_range.offset;
                copy.dstOffset = desc_binding_cmd.desc_set_binding_to_post_process_buffers_lut.offset;
                copy.size = desc_binding_cmd.bound_descriptor_sets.size() * sizeof(VkDeviceAddress);
                DispatchCmdCopyBuffer(per_submission_cb, desc_set_buffer_lut_buffer_range.buffer,
                                      desc_binding_cmd.desc_set_binding_to_post_process_buffers_lut.buffer, 1, &copy);

                VkBufferMemoryBarrier barrier_read_before_write = vku::InitStructHelper();
                barrier_read_before_write.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier_read_before_write.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                barrier_read_before_write.buffer = desc_binding_cmd.desc_set_binding_to_post_process_buffers_lut.buffer;
                barrier_read_before_write.offset = desc_binding_cmd.desc_set_binding_to_post_process_buffers_lut.offset;
                barrier_read_before_write.size = desc_binding_cmd.desc_set_binding_to_post_process_buffers_lut.size;

                DispatchCmdPipelineBarrier(per_submission_cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                                           0, nullptr, 1, &barrier_read_before_write, 0, nullptr);
            }
        }

        // Return post processing lambda, in charge of validating descriptor set accesses done by command buffer submission
        return [bound_desc_sets_to_pp_buffer_map = std::move(bound_desc_sets_to_pp_buffer_map)](
                   Validator& gpuav, CommandBufferSubState& cb, const CommandBufferSubState::LabelLogging& label_logging,
                   const Location& loc) {
            VVL_ZoneScoped;

            // TODO - Currently we don't know the actual call that triggered this, but without just giving "vkCmdDraw" we
            // will get VUID_Undefined We now have the DescriptorValidator::action_index, just need to hook it up!
            Location draw_loc(vvl::Func::vkCmdDraw);

            // We loop each vkCmdBindDescriptorSet, find each VkDescriptorSet that was used in the command buffer, and check
            // its post process buffer for which descriptor was accessed Only check a VkDescriptorSet once, might be bound
            // multiple times in a single command buffer
            for (const auto& [desc_set, pp_buffer_range] : bound_desc_sets_to_pp_buffer_map) {
                // We build once here, but will update the set_index and shader_handle when found
                vvl::DescriptorValidator context(gpuav, cb.base, *desc_set, 0, VK_NULL_HANDLE, nullptr, draw_loc);

                DescriptorAccessMap descriptor_access_map;
                {
                    VVL_ZoneScoped;
                    cb.gpu_resources_manager.InvalidateAllocation(pp_buffer_range);
                    auto slot_ptr = (glsl::PostProcessDescriptorIndexSlot*)pp_buffer_range.offset_mapped_ptr;

                    const std::vector<gpuav::spirv::BindingLayout>& binding_layouts = SubState(*desc_set).GetBindingLayouts();
                    for (uint32_t binding = 0; binding < binding_layouts.size(); binding++) {
                        const gpuav::spirv::BindingLayout& binding_layout = binding_layouts[binding];
                        for (uint32_t descriptor_i = 0; descriptor_i < binding_layout.count; descriptor_i++) {
                            const glsl::PostProcessDescriptorIndexSlot slot = slot_ptr[binding_layout.start + descriptor_i];
                            if (slot.meta_data & glsl::kPostProcessMetaMaskAccessed) {
                                const uint32_t shader_id = slot.meta_data & glsl::kShaderIdMask;
                                const uint32_t action_index = (slot.meta_data & glsl::kPostProcessMetaMaskActionIndex) >>
                                                              glsl::kPostProcessMetaShiftActionIndex;
                                descriptor_access_map[shader_id].emplace_back(
                                    DescriptorAccess{binding, descriptor_i, slot.variable_id, action_index});
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
                        context.SetShaderHandleForGpuAv(&pipeline_state->Handle());
                    } else if (it->second.shader_object != VK_NULL_HANDLE) {
                        shader_object_state = gpuav.Get<vvl::ShaderObject>(it->second.shader_object).get();
                        ASSERT_AND_CONTINUE(shader_object_state->entrypoint);
                        context.SetShaderHandleForGpuAv(&shader_object_state->Handle());
                    } else {
                        assert(false);
                        continue;
                    }

                    for (const DescriptorAccess& descriptor_access : descriptor_accesses) {
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
                            auto variable_it = shader_object_state->entrypoint->resource_interface_variable_map.find(
                                descriptor_access.variable_id);
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

                        // This will represent the Set that was accessed in the shader, which might not match the
                        // vkCmdBindDescriptorSet index if sets are aliased
                        context.SetSetIndexForGpuAv(resource_variable->decorations.set);

                        std::string debug_region_name;
                        if (auto found_label_cmd_i =
                                label_logging.action_cmd_i_to_label_cmd_i_map.find(descriptor_access.action_index);
                            found_label_cmd_i != label_logging.action_cmd_i_to_label_cmd_i_map.end()) {
                            debug_region_name =
                                cb.GetDebugLabelRegion(found_label_cmd_i->second, label_logging.initial_label_stack);
                        }

                        Location access_loc(loc, debug_region_name);
                        context.SetLocationForGpuAv(access_loc);
                        context.ValidateBindingDynamic(*resource_variable, *descriptor_binding, descriptor_access.index);
                    }
                }
            }

            return true;
        };
    });
}

namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_post_process_descriptor_index_comp,
                                             instrumentation_post_process_descriptor_index_comp_size};

const static OfflineFunction kOfflineFunction = {"inst_post_process_descriptor_index",
                                                 instrumentation_post_process_descriptor_index_comp_function_0_offset};

PostProcessDescriptorIndexingPass::PostProcessDescriptorIndexingPass(Module& module) : Pass(module, kOfflineModule) {
    module.use_bda_ = true;
}

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t PostProcessDescriptorIndexingPass::GetLinkFunctionId() { return GetLinkFunction(link_function_id_, kOfflineFunction); }

void PostProcessDescriptorIndexingPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(meta.descriptor_set);
    const Constant& binding_constant = module_.type_manager_.GetConstantUInt32(meta.descriptor_binding);
    const uint32_t descriptor_index_id = CastToUint32(meta.descriptor_index_id, block, inst_it);  // might be int32

    BindingLayout binding_layout = module_.set_index_to_bindings_layout_lut_[meta.descriptor_set][meta.descriptor_binding];
    const Constant& binding_layout_offset = module_.type_manager_.GetConstantUInt32(binding_layout.start);
    const Constant& variable_id_constant = module_.type_manager_.GetConstantUInt32(meta.variable_id);

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t void_type = module_.type_manager_.GetTypeVoid().Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {void_type, function_result, function_def, set_constant.Id(), binding_constant.Id(),
                             descriptor_index_id, binding_layout_offset.Id(), variable_id_constant.Id()},
                            inst_it);
}

bool PostProcessDescriptorIndexingPass::RequiresInstrumentation(const Function& function, const Instruction& inst,
                                                                InstructionMeta& meta) {
    const uint32_t opcode = inst.Opcode();

    const Instruction* var_inst = nullptr;
    if (opcode == spv::OpLoad || opcode == spv::OpStore) {
        const Variable* variable = nullptr;
        const Instruction* access_chain_inst = function.FindInstruction(inst.Operand(0));
        // We need to walk down possibly multiple chained OpAccessChains or OpCopyObject to get the variable
        while (access_chain_inst && access_chain_inst->IsNonPtrAccessChain()) {
            const uint32_t access_chain_base_id = access_chain_inst->Operand(0);
            variable = module_.type_manager_.FindVariableById(access_chain_base_id);
            if (variable) {
                break;  // found
            }
            access_chain_inst = function.FindInstruction(access_chain_base_id);
        }
        if (!variable) {
            return false;
        }
        var_inst = &variable->inst_;

        const uint32_t storage_class = variable->StorageClass();
        if (storage_class != spv::StorageClassUniform && storage_class != spv::StorageClassStorageBuffer) {
            return false;
        }

        const Type* pointer_type = variable->PointerType(module_.type_manager_);
        if (pointer_type->IsArray()) {
            meta.descriptor_index_id = access_chain_inst->Operand(1);
        } else {
            // There is no array of this descriptor, so we essentially have an array of 1
            meta.descriptor_index_id = module_.type_manager_.GetConstantZeroUint32().Id();
        }

    } else {
        // Reference is not load or store, so if it isn't a image-based reference, move on
        const uint32_t image_word = OpcodeImageAccessPosition(opcode);
        if (image_word == 0) {
            return false;
        }
        if (opcode == spv::OpImageTexelPointer || opcode == spv::OpImage) {
            return false;  // need to test if we can support these
        }

        const Instruction* load_inst = function.FindInstruction(inst.Word(image_word));
        while (load_inst && (load_inst->Opcode() == spv::OpSampledImage || load_inst->Opcode() == spv::OpImage ||
                             load_inst->Opcode() == spv::OpCopyObject)) {
            load_inst = function.FindInstruction(load_inst->Operand(0));
        }
        if (!load_inst || load_inst->Opcode() != spv::OpLoad) {
            return false;  // TODO: Handle additional possibilities?
        }

        var_inst = function.FindInstruction(load_inst->Operand(0));
        if (!var_inst) {
            // can be a global variable
            const Variable* global_var = module_.type_manager_.FindVariableById(load_inst->Operand(0));
            var_inst = global_var ? &global_var->inst_ : nullptr;
        }
        if (!var_inst || (!var_inst->IsNonPtrAccessChain() && var_inst->Opcode() != spv::OpVariable)) {
            return false;
        }

        if (var_inst->IsNonPtrAccessChain()) {
            meta.descriptor_index_id = var_inst->Operand(1);

            if (var_inst->Length() > 5) {
                module_.InternalError(Name(), "OpAccessChain has more than 1 indexes");
                return false;
            }

            const Variable* variable = module_.type_manager_.FindVariableById(var_inst->Operand(0));
            if (!variable) {
                module_.InternalError(Name(), "OpAccessChain base is not a variable");
                return false;
            }
            var_inst = &variable->inst_;
        } else {
            meta.descriptor_index_id = module_.type_manager_.GetConstantZeroUint32().Id();
        }
    }

    assert(var_inst);
    meta.variable_id = var_inst->ResultId();
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == meta.variable_id) {
            if (annotation->Word(2) == spv::DecorationDescriptorSet) {
                meta.descriptor_set = annotation->Word(3);
            } else if (annotation->Word(2) == spv::DecorationBinding) {
                meta.descriptor_binding = annotation->Word(3);
            }
        }
    }

    if (meta.descriptor_set >= glsl::kDebugInputBindlessMaxDescSets) {
        module_.InternalWarning(Name(), "Tried to use a descriptor slot over the current max limit");
        return false;
    }

    meta.target_instruction = &inst;

    return true;
}

bool PostProcessDescriptorIndexingPass::Instrument() {
    if (module_.set_index_to_bindings_layout_lut_.empty()) {
        return false;  // If there is no bindings, nothing to instrument
    }

    for (const auto& function : module_.functions_) {
        if (function->instrumentation_added_) continue;

        FunctionDuplicateTracker function_duplicate_tracker;

        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) continue;

            auto& block_instructions = current_block.instructions_;

            // We only need to instrument the set/binding/index/variable combo once per block
            BlockDuplicateTracker& block_duplicate_tracker = function_duplicate_tracker.GetAndUpdate(current_block);
            DescriptroIndexPushConstantAccess pc_access;

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                pc_access.Update(module_, inst_it);

                InstructionMeta meta;
                if (!RequiresInstrumentation(*function, *(inst_it->get()), meta)) {
                    continue;
                }

                const uint32_t hash_descriptor_index_id =
                    pc_access.next_alias_id == meta.descriptor_index_id ? pc_access.descriptor_index_id : meta.descriptor_index_id;
                uint32_t hash_content[4] = {meta.descriptor_set, meta.descriptor_binding, hash_descriptor_index_id,
                                            meta.variable_id};
                const uint32_t hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 4);
                if (function_duplicate_tracker.FindAndUpdate(block_duplicate_tracker, hash)) {
                    continue;  // duplicate detected
                }

                if (IsMaxInstrumentationsCount()) continue;
                instrumentations_count_++;

                CreateFunctionCall(current_block, &inst_it, meta);
            }
        }
    }

    return (instrumentations_count_ != 0);
}

void PostProcessDescriptorIndexingPass::PrintDebugInfo() const {
    std::cout << "PostProcessDescriptorIndexingPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
