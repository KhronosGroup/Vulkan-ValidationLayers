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
    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(meta.descriptor_set);
    const Constant& binding_constant = module_.type_manager_.GetConstantUInt32(meta.descriptor_binding);
    const uint32_t descriptor_index_id = CastToUint32(meta.descriptor_index_id, block, inst_it);  // might be int32

    if (meta.image_inst) {
        const uint32_t opcode = meta.target_instruction->Opcode();
        if (opcode != spv::OpImageRead && opcode != spv::OpImageFetch && opcode != spv::OpImageWrite) {
            // if not a direct read/write/fetch, will be a OpSampledImage
            // "All OpSampledImage instructions must be in the same block in which their Result <id> are consumed"
            // the simple way around this is to add a OpCopyObject to be consumed by the target instruction
            uint32_t image_id = meta.target_instruction->Operand(0);
            const Instruction* sampled_image_inst = block.function_.FindInstruction(image_id);
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
                    block.function_.CreateInstruction(spv::OpCopyObject, {type_id, copy_id, image_id}, image_id);
                }
            }
        }
    }

    BindingLayout binding_layout = module_.set_index_to_bindings_layout_lut_[meta.descriptor_set][meta.descriptor_binding];
    const Constant& binding_layout_size = module_.type_manager_.GetConstantUInt32(binding_layout.count);
    const Constant& binding_layout_offset = module_.type_manager_.GetConstantUInt32(binding_layout.start);

    const uint32_t inst_position = meta.target_instruction->GetPositionIndex();
    const uint32_t inst_position_id = module_.type_manager_.CreateConstantUInt32(inst_position).Id();

    uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId(meta.is_combined_image_sampler);
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_result, function_def, inst_position_id, set_constant.Id(), binding_constant.Id(),
                             descriptor_index_id, binding_layout_size.Id(), binding_layout_offset.Id()},
                            inst_it);

    module_.need_log_error_ = true;

    // If there is a SAMPLER as well, we will inject a second function and combined boolean:
    //     bool valid_image = inst_descriptor_indexing_oob(image);
    //     bool valid_sampler = inst_descriptor_indexing_oob(sampler);
    //     bool valid_both = image_valid && sampler_valid;
    if (meta.sampler_var_inst) {
        const uint32_t valid_image = function_result;
        const uint32_t valid_sampler = module_.TakeNextId();

        const Constant& sampler_set_constant = module_.type_manager_.GetConstantUInt32(meta.sampler_descriptor_set);
        const Constant& sampler_binding_constant = module_.type_manager_.GetConstantUInt32(meta.sampler_descriptor_binding);
        const uint32_t sampler_descriptor_index_id =
            CastToUint32(meta.sampler_descriptor_index_id, block, inst_it);  // might be int32

        BindingLayout sampler_binding_layout =
            module_.set_index_to_bindings_layout_lut_[meta.sampler_descriptor_set][meta.sampler_descriptor_binding];
        const Constant& sampler_binding_layout_size = module_.type_manager_.GetConstantUInt32(sampler_binding_layout.count);
        const Constant& sampler_binding_layout_offset = module_.type_manager_.GetConstantUInt32(sampler_binding_layout.start);

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
    const uint32_t opcode = inst.Opcode();

    bool array_found = false;
    const Instruction* sampler_load_inst = nullptr;
    if (opcode == spv::OpAtomicLoad || opcode == spv::OpAtomicStore || opcode == spv::OpAtomicExchange) {
        // Image Atomics
        const Instruction* image_texel_ptr_inst = function.FindInstruction(inst.Operand(0));
        if (!image_texel_ptr_inst || image_texel_ptr_inst->Opcode() != spv::OpImageTexelPointer) {
            return false;
        }

        const Variable* variable = nullptr;
        const Instruction* access_chain_inst = function.FindInstruction(image_texel_ptr_inst->Operand(0));
        if (access_chain_inst) {
            variable = module_.type_manager_.FindVariableById(access_chain_inst->Operand(0));
        } else {
            // if no array, will point right to a variable
            variable = module_.type_manager_.FindVariableById(image_texel_ptr_inst->Operand(0));
        }

        if (!variable) {
            return false;
        }
        meta.var_inst = &variable->inst_;

        const Type* pointer_type = variable->PointerType(module_.type_manager_);
        if (!pointer_type) {
            module_.InternalError(Name(), "Pointer type not found");
            return false;
        }

        const bool non_empty_access_chain = access_chain_inst && access_chain_inst->Length() >= 5;
        if (pointer_type->IsArray() && non_empty_access_chain) {
            array_found = true;
            meta.descriptor_index_id = access_chain_inst->Operand(1);
        } else {
            // There is no array of this descriptor, so we essentially have an array of 1
            meta.descriptor_index_id = module_.type_manager_.GetConstantZeroUint32().Id();
        }
    } else if (opcode == spv::OpLoad || opcode == spv::OpStore || AtomicOperation(opcode)) {
        // Buffer and Buffer Atomics and Storage Images

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

        meta.var_inst = &variable->inst_;

        const uint32_t storage_class = variable->StorageClass();
        if (storage_class == spv::StorageClassUniformConstant) {
            // TODO - Need to add Storage Image support
            return false;
        }
        if (storage_class != spv::StorageClassUniform && storage_class != spv::StorageClassStorageBuffer) {
            return false;  // Prevents things like Push Constants
        }

        const Type* pointer_type = variable->PointerType(module_.type_manager_);
        if (!pointer_type) {
            module_.InternalError(Name(), "Pointer type not found");
            return false;
        }

        if (pointer_type->IsArray()) {
            array_found = true;
            meta.descriptor_index_id = access_chain_inst->Operand(1);
        } else {
            // There is no array of this descriptor, so we essentially have an array of 1
            meta.descriptor_index_id = module_.type_manager_.GetConstantZeroUint32().Id();
        }
    } else {
        // sampled image (non-atomic)

        // Reference is not load or store, so if it isn't a image-based reference, move on
        const uint32_t image_word = OpcodeImageAccessPosition(opcode);
        if (image_word == 0) {
            return false;
        }

        // Things that have an OpImage (in OpcodeImageAccessPosition) but we don't want to handle
        if (opcode == spv::OpImageRead || opcode == spv::OpImageWrite) {
            return false;  // Storage Images are handled at OpLoad
        } else if (opcode == spv::OpImageTexelPointer) {
            return false;  // atomics are handled separately
        } else if (opcode == spv::OpImage) {
            return false;  // Don't deal with the access directly
        }

        meta.image_inst = function.FindInstruction(inst.Word(image_word));
        const Instruction* load_inst = meta.image_inst;
        while (load_inst && (load_inst->Opcode() == spv::OpSampledImage || load_inst->Opcode() == spv::OpImage ||
                             load_inst->Opcode() == spv::OpCopyObject)) {
            load_inst = function.FindInstruction(load_inst->Operand(0));
            if (load_inst->Opcode() == spv::OpSampledImage) {
                sampler_load_inst = function.FindInstruction(load_inst->Operand(1));
            }
        }

        // If we can't find a seperate sampler, and non sampled images are check elsewhere, we know this is actually a combined
        // image sampler
        meta.is_combined_image_sampler = sampler_load_inst == nullptr;

        if (!load_inst || load_inst->Opcode() != spv::OpLoad) {
            return false;  // TODO: Handle additional possibilities?
        }

        meta.var_inst = function.FindInstruction(load_inst->Operand(0));
        if (!meta.var_inst) {
            // can be a global variable
            const Variable* global_var = module_.type_manager_.FindVariableById(load_inst->Operand(0));
            meta.var_inst = global_var ? &global_var->inst_ : nullptr;
        }
        if (!meta.var_inst || (!meta.var_inst->IsNonPtrAccessChain() && meta.var_inst->Opcode() != spv::OpVariable)) {
            return false;
        }

        if (meta.var_inst->IsNonPtrAccessChain()) {
            array_found = true;
            meta.descriptor_index_id = meta.var_inst->Operand(1);

            if (meta.var_inst->Length() > 5) {
                module_.InternalError(Name(), "OpAccessChain has more than 1 indexes");
                return false;
            }

            const Variable* variable = module_.type_manager_.FindVariableById(meta.var_inst->Operand(0));
            if (!variable) {
                module_.InternalError(Name(), "OpAccessChain base is not a variable");
                return false;
            }
            meta.var_inst = &variable->inst_;
        } else {
            meta.descriptor_index_id = module_.type_manager_.GetConstantZeroUint32().Id();
        }
    }

    // guaranteed to be valid already, save compiler time optimizing the check out
    if (!array_found && !module_.has_bindless_descriptors_) {
        return false;
    }

    assert(meta.var_inst);
    uint32_t variable_id = meta.var_inst->ResultId();
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == variable_id) {
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

    if (!module_.settings_.safe_mode) {
        auto variable_found_it = block_instrumented_table_.find(variable_id);
        if (variable_found_it == block_instrumented_table_.end()) {
            block_instrumented_table_[variable_id] = {meta.descriptor_index_id};
        } else {
            vvl::unordered_set<uint32_t>& descriptor_index_set = variable_found_it->second;
            if (descriptor_index_set.find(meta.descriptor_index_id) != descriptor_index_set.end()) {
                return false;  // Already instrumented, can skip
            } else {
                descriptor_index_set.emplace(meta.descriptor_index_id);
            }
        }
    }

    // When using a SAMPLED_IMAGE and SAMPLER, they are accessed together so we need check for 2 descriptors at the same time
    // TODO - This is currently 95% the same logic as above, find a way to combine it
    if (sampler_load_inst && sampler_load_inst->Opcode() == spv::OpLoad) {
        meta.sampler_var_inst = function.FindInstruction(sampler_load_inst->Operand(0));
        if (!meta.sampler_var_inst) {
            // can be a global variable
            const Variable* global_var = module_.type_manager_.FindVariableById(sampler_load_inst->Operand(0));
            meta.sampler_var_inst = global_var ? &global_var->inst_ : nullptr;
        }
        if (!meta.sampler_var_inst ||
            (!meta.sampler_var_inst->IsNonPtrAccessChain() && meta.sampler_var_inst->Opcode() != spv::OpVariable)) {
            return false;
        }

        if (meta.sampler_var_inst->IsNonPtrAccessChain()) {
            array_found = true;
            meta.sampler_descriptor_index_id = meta.sampler_var_inst->Operand(1);

            if (meta.sampler_var_inst->Length() > 5) {
                module_.InternalError(Name(), "Sampler OpAccessChain has more than 1 indexes");
                return false;
            }

            const Variable* variable = module_.type_manager_.FindVariableById(meta.sampler_var_inst->Operand(0));
            if (!variable) {
                module_.InternalError(Name(), "Sampler OpAccessChain base is not a variable");
                return false;
            }
            meta.sampler_var_inst = &variable->inst_;
        } else {
            meta.sampler_descriptor_index_id = module_.type_manager_.GetConstantZeroUint32().Id();
        }

        variable_id = meta.sampler_var_inst->ResultId();
        for (const auto& annotation : module_.annotations_) {
            if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == variable_id) {
                if (annotation->Word(2) == spv::DecorationDescriptorSet) {
                    meta.sampler_descriptor_set = annotation->Word(3);
                } else if (annotation->Word(2) == spv::DecorationBinding) {
                    meta.sampler_descriptor_binding = annotation->Word(3);
                }
            }
        }

        if (meta.sampler_descriptor_set >= glsl::kDebugInputBindlessMaxDescSets) {
            module_.InternalWarning(Name(), "Sampler Tried to use a descriptor slot over the current max limit");
            return false;
        }
    }
    // Save information to be used to make the Function
    meta.target_instruction = &inst;

    return true;
}
bool DescriptorIndexingOOBPass::Instrument() {
    if (module_.set_index_to_bindings_layout_lut_.empty()) {
        return false;  // If there is no bindings, nothing to instrument
    }

    // Due to the current way we use iterators, we will actually create new blocks when placing the conditional functions
    // Need a way to convey if the new block is a "real" true new block, or just the rest of the one we split up
    bool is_original_new_block = true;

    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (const auto& function : module_.functions_) {
        if (function->instrumentation_added_) continue;

        FunctionDuplicateTracker function_duplicate_tracker;

        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            BasicBlock& current_block = **block_it;

            cf_.Update(current_block);
            if (debug_disable_loops_ && cf_.in_loop) continue;

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
                if (module_.settings_.safe_mode) {
                    pc_access.Update(module_, inst_it);
                }

                InstructionMeta meta;
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(*function, *(inst_it->get()), meta)) {
                    // TODO - This should be cleaned up then having it injected here
                    // we can have a situation where the incoming SPIR-V looks like
                    // %a = OpSampledImage %type %image %sampler
                    // ... other stuff we inject a
                    // function around
                    // %b = OpImageSampleExplicitLod %type2 %a %3893 Lod %3918
                    // and we get an error "All OpSampledImage instructions must be in the same block in which their Result <id> are
                    // consumed" to get around this we inject a OpCopyObject right after the OpSampledImage
                    if ((*inst_it)->Opcode() == spv::OpSampledImage) {
                        const uint32_t result_id = (*inst_it)->ResultId();
                        const uint32_t type_id = (*inst_it)->TypeId();
                        const uint32_t copy_id = module_.TakeNextId();
                        function->ReplaceAllUsesWith(result_id, copy_id);
                        inst_it++;
                        current_block.CreateInstruction(spv::OpCopyObject, {type_id, copy_id, result_id}, &inst_it);
                        inst_it--;
                    }
                    continue;
                }

                if (!module_.settings_.safe_mode) {
                    const uint32_t hash_descriptor_index_id = pc_access.next_alias_id == meta.descriptor_index_id
                                                                  ? pc_access.descriptor_index_id
                                                                  : meta.descriptor_index_id;
                    uint32_t hash_content[3] = {meta.descriptor_set, meta.descriptor_binding, hash_descriptor_index_id};
                    const uint32_t hash = hash_util::Hash32(hash_content, sizeof(uint32_t) * 3);
                    if (function_duplicate_tracker.FindAndUpdate(block_duplicate_tracker, hash)) {
                        continue;  // duplicate detected
                    }
                }

                if (IsMaxInstrumentationsCount()) continue;
                instrumentations_count_++;

                if (!module_.settings_.safe_mode) {
                    CreateFunctionCall(current_block, &inst_it, meta);
                } else {
                    InjectConditionalData ic_data = InjectFunctionPre(*function.get(), block_it, inst_it);
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