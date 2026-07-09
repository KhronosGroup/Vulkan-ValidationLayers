/* Copyright (c) 2024-2026 LunarG, Inc.
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

#include "descriptor_indexing_oob_pass.h"
#include "link.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/gpuav_offline_spirv.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "utils/hash_util.h"

namespace gpuav {
namespace spirv {

const static OfflineModule kOfflineModule = {instrumentation_descriptor_indexing_oob_comp,
                                             instrumentation_descriptor_indexing_oob_comp_size, UseErrorPayloadVariable};

// Non-Bindless is simpler and we want to use when possible
const static OfflineFunction kOfflineFunctionNonBindless = {"inst_descriptor_indexing_oob_non_bindless",
                                                            instrumentation_descriptor_indexing_oob_comp_function_0_offset};
const static OfflineFunction kOfflineFunctionBindless = {"inst_descriptor_indexing_oob_bindless",
                                                         instrumentation_descriptor_indexing_oob_comp_function_1_offset};
const static OfflineFunction kOfflineFunctionBindlessCombined = {"inst_descriptor_indexing_oob_bindless_combined_image_sampler",
                                                                 instrumentation_descriptor_indexing_oob_comp_function_2_offset};

DescriptorIndexingOOBPass::DescriptorIndexingOOBPass(Module& module) : Pass(module, kOfflineModule) { module.use_bda_ = true; }

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t DescriptorIndexingOOBPass::GetLinkFunctionId(bool is_combined_image_sampler) {
    if (!module_.has_bindless_descriptors_) {
        return GetLinkFunction(link_non_bindless_id_, kOfflineFunctionNonBindless);
    } else if (is_combined_image_sampler) {
        return GetLinkFunction(link_bindless_combined_image_sampler_id_, kOfflineFunctionBindlessCombined);
    } else {
        return GetLinkFunction(link_bindless_id_, kOfflineFunctionBindless);
    }
}

uint32_t DescriptorIndexingOOBPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InstructionMeta& meta) {
    // TODO - This logic is not obvious and should be either fixed or moved to a common util
    // If dealing with a seperate sampler, only need to do it on the resource
    // To add to the fire, this needs to go first otherwise the Function::CreateInstruction will break the inst_it
    if (meta.access_path.image_load_inst) {
        const uint32_t opcode = meta.target_instruction->Opcode();
        if (opcode != spv::OpImageRead && opcode != spv::OpImageFetch && opcode != spv::OpImageWrite) {
            // if not a direct read/write/fetch, will be a OpSampledImage
            // "All OpSampledImage instructions must be in the same block in which their Result <id> are consumed"
            // the simple way around this is to add a OpCopyObject to be consumed by the target instruction
            uint32_t image_id = meta.target_instruction->Operand(0);
            const Instruction* sampled_image_inst = block.function_->FindInstruction(image_id);
            // TODO - Add tests to understand what else can be here other then OpSampledImage
            if (sampled_image_inst->Opcode() == spv::OpSampledImage) {
                const uint32_t type_id = sampled_image_inst->TypeId();
                const uint32_t copy_id = module_.TakeNextId();
                const_cast<Instruction*>(meta.target_instruction)->ReplaceOperandId(image_id, copy_id);

                // incase the OpSampledImage is shared, copy the previous OpCopyObject
                auto copied = copy_object_map_.find(image_id);
                if (copied != copy_object_map_.end()) {
                    image_id = copied->second;
                    block.CreateInstruction(spv::OpCopyObject, {type_id, copy_id, image_id}, inst_it);
                } else {
                    copy_object_map_.emplace(image_id, copy_id);
                    // slower, but need to guarantee it is placed after a OpSampledImage
                    block.function_->CreateInstruction(spv::OpCopyObject, {type_id, copy_id, image_id}, image_id, inst_it);
                }
            }
        }
    }

    const DescriptorInterface& interface = meta.access_path.variable->interface_;
    const Constant& set_constant = type_manager_.GetConstantUInt32(interface.set);
    const Constant& binding_constant = type_manager_.GetConstantUInt32(interface.binding);
    const uint32_t descriptor_index_id = CastToUint32(meta.access_path.descriptor_index_id, block, inst_it);  // might be int32

    const auto& layout_lut = module_.interface_.instrumentation_dsl.set_index_to_bindings_layout_lut;
    BindingLayout binding_layout = layout_lut[interface.set][interface.binding];
    const Constant& binding_layout_size = type_manager_.GetConstantUInt32(binding_layout.count);
    const Constant& binding_layout_offset = type_manager_.GetConstantUInt32(binding_layout.start);

    const uint32_t inst_position = meta.target_instruction->GetPositionOffset();
    const uint32_t inst_position_id = type_manager_.CreateConstantUInt32(inst_position).Id();

    uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId(meta.access_path.is_combined_image_sampler);
    const uint32_t bool_type = type_manager_.GetTypeBool().Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_result, function_def, inst_position_id, set_constant.Id(), binding_constant.Id(),
                             descriptor_index_id, binding_layout_size.Id(), binding_layout_offset.Id()},
                            inst_it);

    module_.need_log_error_ = true;

    // If there is a SAMPLER as well, we will inject a second function and combined boolean:
    //     bool valid_image = inst_descriptor_indexing_oob(image);
    //     bool valid_sampler = inst_descriptor_indexing_oob(sampler);
    //     bool valid_both = image_valid && sampler_valid;
    if (meta.access_path.sampler_variable) {
        const uint32_t valid_image = function_result;
        const uint32_t valid_sampler = module_.TakeNextId();
        const DescriptorInterface& sampler_interface = meta.access_path.sampler_variable->interface_;

        const Constant& sampler_set_constant = type_manager_.GetConstantUInt32(sampler_interface.set);
        const Constant& sampler_binding_constant = type_manager_.GetConstantUInt32(sampler_interface.binding);
        const uint32_t sampler_descriptor_index_id = CastToUint32(meta.access_path.sampler_descriptor_index_id, block, inst_it);

        BindingLayout sampler_binding_layout = layout_lut[sampler_interface.set][sampler_interface.binding];
        const Constant& sampler_binding_layout_size = type_manager_.GetConstantUInt32(sampler_binding_layout.count);
        const Constant& sampler_binding_layout_offset = type_manager_.GetConstantUInt32(sampler_binding_layout.start);

        block.CreateInstruction(
            spv::OpFunctionCall,
            {bool_type, valid_sampler, function_def, inst_position_id, sampler_set_constant.Id(), sampler_binding_constant.Id(),
             sampler_descriptor_index_id, sampler_binding_layout_size.Id(), sampler_binding_layout_offset.Id()},
            inst_it);

        function_result = module_.TakeNextId();  // valid_both
        block.CreateInstruction(spv::OpLogicalAnd, {bool_type, function_result, valid_image, valid_sampler}, inst_it);
    }

    return function_result;
}

bool DescriptorIndexingOOBPass::RequiresInstrumentation(const Function& function, const Instruction& inst, InstructionMeta& meta) {
    meta.access_path = type_manager_.BuildAccessPath(function, inst);
    if (!meta.access_path.IsValid() || !meta.access_path.variable->IsDescriptor()) {
        return false;
    }

    // guaranteed to be valid already, save compiler time optimizing the check out
    if (!meta.access_path.pointer_type->IsArray() && !module_.has_bindless_descriptors_) {
        return false;
    }

    if (meta.access_path.variable->interface_.set >= glsl::kDebugInputBindlessMaxDescSets) {
        module_.InternalWarning(Name(), "Tried to use a descriptor slot over the current max limit");
        return false;
    }

    if (!module_.settings_.safe_mode) {
        uint32_t variable_id = meta.access_path.variable->Id();
        auto variable_found_it = block_instrumented_table_.find(variable_id);
        if (variable_found_it == block_instrumented_table_.end()) {
            block_instrumented_table_[variable_id] = {meta.access_path.descriptor_index_id};
        } else {
            vvl::unordered_set<uint32_t>& descriptor_index_set = variable_found_it->second;
            if (descriptor_index_set.find(meta.access_path.descriptor_index_id) != descriptor_index_set.end()) {
                return false;  // Already instrumented, can skip
            } else {
                descriptor_index_set.emplace(meta.access_path.descriptor_index_id);
            }
        }
    }

    if (meta.access_path.sampler_variable &&
        meta.access_path.sampler_variable->interface_.set >= glsl::kDebugInputBindlessMaxDescSets) {
        module_.InternalWarning(Name(), "Sampler Tried to use a descriptor slot over the current max limit");
        return false;
    }

    meta.target_instruction = &inst;

    return true;
}
bool DescriptorIndexingOOBPass::Instrument() {
    if (module_.interface_.instrumentation_dsl.set_index_to_bindings_layout_lut.empty()) {
        return false;  // If there is no bindings, nothing to instrument
    }

    // Due to the current way we use iterators, we will actually create new blocks when placing the conditional functions
    // Need a way to convey if the new block is a "real" true new block, or just the rest of the one we split up
    bool is_original_new_block = true;

    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (Function& function : module_.functions_) {
        if (!function.called_from_target_) {
            continue;
        }

        FunctionDuplicateTracker function_duplicate_tracker;

        for (auto block_it = function.blocks_.begin(); block_it != function.blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) {
                continue;
            }

            if (current_block.IsLoopHeader()) {
                continue;  // Currently can't properly handle injecting CFG logic into a loop header block
            }
            auto& block_instructions = current_block.instructions_;

            // Don't clear if the new block occurs from control flow breaking one up
            if (is_original_new_block) {
                block_instrumented_table_.clear();
            }
            is_original_new_block = true;  // Always reset once we start

            // We only need to instrument the set/binding/index combo once per block (in unsafe mode)
            BlockDuplicateTracker& block_duplicate_tracker = function_duplicate_tracker.GetAndUpdate(current_block);
            DescriptroIndexPushConstantAccess pc_access;

            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                if (!module_.settings_.safe_mode) {
                    pc_access.Update(module_, inst_it);
                }

                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(function, *(inst_it->get()), meta)) {
                    // TODO - This should be cleaned up then having it injected here
                    // we can have a situation where the incoming SPIR-V looks like
                    // %a = OpSampledImage %type %image %sampler
                    // ... other stuff we inject a function around
                    // %b = OpImageSampleExplicitLod %type2 %a %3893 Lod %3918
                    // and we get an error "All OpSampledImage instructions must be in the same block in which their Result <id> are
                    // consumed" to get around this we inject a OpCopyObject right after the OpSampledImage
                    //
                    // https://github.com/KhronosGroup/SPIRV-Tools/issues/5830
                    if ((*inst_it)->Opcode() == spv::OpSampledImage) {
                        const uint32_t result_id = (*inst_it)->ResultId();
                        const uint32_t type_id = (*inst_it)->TypeId();
                        const uint32_t copy_id = module_.TakeNextId();
                        function.ReplaceAllUsesWith(result_id, copy_id);
                        inst_it++;
                        current_block.CreateInstruction(spv::OpCopyObject, {type_id, copy_id, result_id}, &inst_it);
                        inst_it--;
                    }
                    continue;
                }

                if (!module_.settings_.safe_mode) {
                    const uint32_t hash_descriptor_index_id = pc_access.next_alias_id == meta.access_path.descriptor_index_id
                                                                  ? pc_access.descriptor_index_id
                                                                  : meta.access_path.descriptor_index_id;
                    uint32_t hash_content[3] = {meta.access_path.variable->interface_.set,
                                                meta.access_path.variable->interface_.binding, hash_descriptor_index_id};
                    const uint32_t hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 3);
                    if (function_duplicate_tracker.FindAndUpdate(block_duplicate_tracker, hash)) {
                        continue;  // duplicate detected
                    }
                }

                if (MaxInstrumentationsCountReached()) {
                    return instrumentations_count_ != 0;
                }
                instrumentations_count_++;

                if (!module_.settings_.safe_mode) {
                    CreateFunctionCall(current_block, &inst_it, meta);
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(function, block_it, inst_it);
                    ic_data.function_result_id = CreateFunctionCall(current_block, nullptr, meta);
                    InjectFunctionPost(current_block, ic_data);
                    // Skip the newly added valid and invalid block. Start searching again from newly split merge block
                    block_it++;
                    block_it++;
                    is_original_new_block = false;
                    break;
                }
            }
        }
    }

    return instrumentations_count_ != 0;
}

void DescriptorIndexingOOBPass::PrintDebugInfo() const {
    std::cout << "DescriptorIndexingOOBPass instrumentation count: " << instrumentations_count_ << " ("
              << (module_.has_bindless_descriptors_ ? "Bindless version" : "Non Bindless version") << ")\n";
}

}  // namespace spirv
}  // namespace gpuav