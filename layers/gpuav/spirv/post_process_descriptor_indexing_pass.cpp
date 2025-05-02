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

#include "generated/gpuav_offline_spirv.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "utils/hash_util.h"

namespace gpuav {
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
