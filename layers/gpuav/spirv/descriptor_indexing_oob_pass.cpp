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

#include "generated/instrumentation_descriptor_indexing_oob_bindless_comp.h"
#include "generated/instrumentation_descriptor_indexing_oob_bindless_combined_image_sampler_comp.h"
#include "generated/instrumentation_descriptor_indexing_oob_non_bindless_comp.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"

namespace gpuav {
namespace spirv {

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t DescriptorIndexingOOBPass::GetLinkFunctionId(bool is_combined_image_sampler) {
    // This pass has 2 variations of GLSL we can pull in. Non-Bindless is simpler and we want to use when possible
    static LinkInfo link_info_bindless = {instrumentation_descriptor_indexing_oob_bindless_comp,
                                          instrumentation_descriptor_indexing_oob_bindless_comp_size, 0,
                                          "inst_descriptor_indexing_oob_bindless"};

    static LinkInfo link_info_bindless_combined_image_sampler = {
        instrumentation_descriptor_indexing_oob_bindless_combined_image_sampler_comp,
        instrumentation_descriptor_indexing_oob_bindless_combined_image_sampler_comp_size, 0,
        "inst_descriptor_indexing_oob_bindless_combined_image_sampler"};

    static LinkInfo link_info_non_bindless = {instrumentation_descriptor_indexing_oob_non_bindless_comp,
                                              instrumentation_descriptor_indexing_oob_non_bindless_comp_size, 0,
                                              "inst_descriptor_indexing_oob_non_bindless"};

    uint32_t link_id = 0;
    if (!module_.has_bindless_descriptors_) {
        if (link_function_id_non_bindless_ == 0) {
            link_function_id_non_bindless_ = module_.TakeNextId();
            LinkInfo& link_info = link_info_non_bindless;
            link_info.function_id = link_function_id_non_bindless_;
            module_.link_info_.push_back(link_info);
        }
        link_id = link_function_id_non_bindless_;
    } else if (is_combined_image_sampler) {
        if (link_function_id_bindless_combined_image_sampler_ == 0) {
            link_function_id_bindless_combined_image_sampler_ = module_.TakeNextId();
            LinkInfo& link_info = link_info_bindless_combined_image_sampler;
            link_info.function_id = link_function_id_bindless_combined_image_sampler_;
            module_.link_info_.push_back(link_info);
        }
        link_id = link_function_id_bindless_combined_image_sampler_;
    } else {
        if (link_function_id_bindless_ == 0) {
            link_function_id_bindless_ = module_.TakeNextId();
            LinkInfo& link_info = link_info_bindless;
            link_info.function_id = link_function_id_bindless_;
            module_.link_info_.push_back(link_info);
        }
        link_id = link_function_id_bindless_;
    }
    return link_id;
}

uint32_t DescriptorIndexingOOBPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it,
                                                       const InjectionData& injection_data, const InstructionMeta& meta) {
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

    uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId(meta.is_combined_image_sampler);
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    block.CreateInstruction(
        spv::OpFunctionCall,
        {bool_type, function_result, function_def, injection_data.inst_position_id, injection_data.stage_info_id, set_constant.Id(),
         binding_constant.Id(), descriptor_index_id, binding_layout_size.Id(), binding_layout_offset.Id()},
        inst_it);

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

        block.CreateInstruction(spv::OpFunctionCall,
                                {bool_type, valid_sampler, function_def, injection_data.inst_position_id,
                                 injection_data.stage_info_id, sampler_set_constant.Id(), sampler_binding_constant.Id(),
                                 sampler_descriptor_index_id, sampler_binding_layout_size.Id(), sampler_binding_layout_offset.Id()},
                                inst_it);

        function_result = module_.TakeNextId();  // valid_both
        block.CreateInstruction(spv::OpLogicalAnd, {bool_type, function_result, valid_image, valid_sampler}, inst_it);
    }

    return function_result;
}

bool DescriptorIndexingOOBPass::EarlySkip() const {
    if (module_.set_index_to_bindings_layout_lut_.empty()) {
        return true;  // If there is no bindings, nothing to instrument
    }
    return false;
}

void DescriptorIndexingOOBPass::NewBlock(const BasicBlock&, bool is_original_new_block) {
    // Don't clear if the new block occurs from control flow breaking one up
    if (is_original_new_block) {
        block_instrumented_table_.clear();
    }
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
        while (access_chain_inst && access_chain_inst->Opcode() == spv::OpAccessChain) {
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
        if (!meta.var_inst || (meta.var_inst->Opcode() != spv::OpAccessChain && meta.var_inst->Opcode() != spv::OpVariable)) {
            return false;
        }

        if (meta.var_inst->Opcode() == spv::OpAccessChain) {
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

    if (module_.settings_.unsafe_mode) {
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
            (meta.sampler_var_inst->Opcode() != spv::OpAccessChain && meta.sampler_var_inst->Opcode() != spv::OpVariable)) {
            return false;
        }

        if (meta.sampler_var_inst->Opcode() == spv::OpAccessChain) {
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

void DescriptorIndexingOOBPass::PrintDebugInfo() const {
    std::cout << "DescriptorIndexingOOBPass instrumentation count: " << instrumentations_count_ << " ("
              << (module_.has_bindless_descriptors_ ? "Bindless version" : "Non Bindless version") << ")\n";
}

}  // namespace spirv
}  // namespace gpuav